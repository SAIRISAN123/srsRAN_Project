/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "ue.h"

namespace srsran {

/// \brief This class assists with the search of PDSCH parameters given an UE config that ensures a valid UE PDSCH
/// resource allocation in the Cell Resource Grid. The search iterates through different HARQ, SearchSpace and PDSCH
/// time-domain resource candidates, returning only the valid ones. This class operates as a range with begin()
/// and end() iterators, and only searches for valid candidates in a lazy fashion. That means that we only compute
/// all valid candidates if we iterate from begin() to end().
///
/// To be a valid candidate, the following conditions must be met:
/// - The HARQ process state must match the \c is_retx state passed in the constructor. If \c is_retx is false, there
/// should be at most one empty HARQ candidate returned during the whole search. If \c is_retx is true, all the HARQs
/// of the candidates returned must have a pending retransmission.
/// - We avoid SearchSpace#0 for UE PDSCH allocation.
/// - The slot must have sufficient DL symbols, given the PDSCH time-domain resource.
/// - For \c is_retx equal to true, we ensure the candidates will lead to allocations with the same number of DL
/// symbols as the previous HARQ allocation.
class ue_pdsch_param_candidate_searcher
{
  using search_space_candidate_list = static_vector<const search_space_info*, MAX_NOF_SEARCH_SPACE_PER_BWP>;
  using dl_harq_iter                = static_vector<const dl_harq_process*, MAX_NOF_HARQS>::const_iterator;
  using ss_iter = static_vector<const search_space_info*, MAX_NOF_SEARCH_SPACE_PER_BWP>::const_iterator;

public:
  struct iterator;

  /// Parameters for a given PDSCH candidate.
  struct candidate {
    /// DL HARQ process (may be newTx or reTx).
    const dl_harq_process& harq() const { return **harq_it; }

    /// Chosen Search Space.
    const search_space_info& ss() const { return **ss_it; }

    /// Chosen PDSCH Time Domain Resource.
    const pdsch_time_domain_resource_allocation& pdsch_td_res() const
    {
      return (*ss_it)->pdsch_time_domain_list[time_res];
    }

    /// Chosen PDSCH Time Domain Resource Index.
    unsigned pdsch_td_res_index() const { return time_res; }

    bool operator==(const candidate& other) const
    {
      return harq_it == other.harq_it and ss_it == other.ss_it and time_res == other.time_res;
    }
    bool operator!=(const candidate& other) const { return not(*this == other); }
    /// A candidate being less than another means that the first has higher priority than the second.
    bool operator<(const candidate& other) const
    {
      return harq_it < other.harq_it or (harq_it == other.harq_it and ss_it < other.ss_it) or
             (harq_it == other.harq_it and ss_it == other.ss_it and time_res < other.time_res);
    }
    bool operator>=(const candidate& other) const { return not(*this < other); }
    bool operator<=(const candidate& other) const { return *this < other or *this == other; }
    bool operator>(const candidate& other) const { return not(*this < other) and *this != other; }

  private:
    friend struct iterator;
    friend class ue_pdsch_param_candidate_searcher;

    candidate(dl_harq_iter h, ss_iter ss, unsigned time_res_) : harq_it(h), ss_it(ss), time_res(time_res_) {}

    dl_harq_iter harq_it;
    ss_iter      ss_it;
    unsigned     time_res;
  };

  /// Iterator for list of candidates.
  struct iterator {
    using value_type = candidate;
    using reference  = value_type&;
    using pointer    = value_type*;

    iterator(ue_pdsch_param_candidate_searcher& parent_, dl_harq_iter h_it, ss_iter ss_it, unsigned time_res_) :
      parent(&parent_), current(h_it, ss_it, time_res_)
    {
      parent->iterate_until_valid_candidate_found(current);
    }

    candidate&       operator*() { return current; }
    const candidate& operator*() const { return current; }
    candidate*       operator->() { return &current; }
    const candidate* operator->() const { return &current; }

    /// Incrementing the iterator means moving up one position in the list of candidates.
    iterator& operator++()
    {
      ++current.time_res;
      // When we increment the time_res, it may be pointing to the end of the PDSCH time domain resource list.
      // So, we may need to move to the next SearchSpace or HARQ candidate.
      parent->iterate_until_valid_candidate_found(current);
      return *this;
    }

    bool operator==(const iterator& other) const
    {
      return parent == other.parent and
             ((current.harq_it == parent->dl_harq_candidates.end() and current.harq_it == other.current.harq_it) or
              (current == other.current));
    }
    bool operator!=(const iterator& other) const { return not(*this == other); }

  private:
    ue_pdsch_param_candidate_searcher* parent;
    candidate                          current;
  };

  /// Create a searcher for UE PDSCH parameters.
  ue_pdsch_param_candidate_searcher(const ue&       ue_ref_,
                                    ue_cell_index_t cell_index,
                                    bool            is_retx_,
                                    slot_point      pdcch_slot_) :
    ue_ref(ue_ref_), ue_cc(ue_ref.get_cell(cell_index)), is_retx(is_retx_), pdcch_slot(pdcch_slot_)
  {
    if (is_retx) {
      // Create list of DL HARQ processes with pending retx, sorted from oldest to newest.
      for (unsigned i = 0; i != ue_cc.harqs.nof_dl_harqs(); ++i) {
        const dl_harq_process& h = ue_cc.harqs.dl_harq(i);
        if (h.has_pending_retx()) {
          dl_harq_candidates.push_back(&h);
        }
      }
      std::sort(
          dl_harq_candidates.begin(),
          dl_harq_candidates.end(),
          [](const dl_harq_process* lhs, const dl_harq_process* rhs) { return lhs->slot_ack() < rhs->slot_ack(); });
    } else {
      // If there are no pending new Tx bytes, return.
      if (not ue_ref.has_pending_dl_newtx_bytes()) {
        return;
      }

      // Find empty HARQ. Create a list with a single position.
      const dl_harq_process* h = ue_cc.harqs.find_empty_dl_harq();
      if (h != nullptr) {
        dl_harq_candidates.push_back(h);
      }
    }
  }

  /// Get begin to the list of candidates.
  iterator begin() { return iterator{*this, dl_harq_candidates.begin(), ss_candidate_list.begin(), 0}; }
  iterator end() { return iterator{*this, dl_harq_candidates.end(), nullptr, 0}; }

  /// List of DL HARQ candidates.
  const static_vector<const dl_harq_process*, MAX_NOF_HARQS>& dl_harqs() const { return dl_harq_candidates; }

private:
  // Generate Search Space candidates for a given HARQ.
  void generate_ss_candidates(dl_harq_iter current_harq_it)
  {
    if (harq_of_ss_list == *current_harq_it) {
      // The HARQ candidate didn't change. We can early exit.
      return;
    }
    srsran_assert(is_retx or harq_of_ss_list == nullptr, "Regenerating SS candidates should only be needed for reTxs");

    // New HARQ. Search Space candidates are recomputed.
    const auto* prev_h = harq_of_ss_list;
    harq_of_ss_list    = *current_harq_it;

    // Check which RNTI Type is preferred for this HARQ.
    optional<dci_dl_rnti_config_type> preferred_rnti_type;
    if (is_retx) {
      preferred_rnti_type = harq_of_ss_list->last_alloc_params().dci_cfg_type;
    }

    if (prev_h != nullptr and preferred_rnti_type == prev_h->last_alloc_params().dci_cfg_type) {
      // It is the same RNTI Type as the previous HARQ candidate. Search Space Candidate doesn't need to be regenerated,
      // and we can use the previous list.
      return;
    }

    // Update alloc_params list.
    ss_candidate_list = ue_cc.get_active_dl_search_spaces(preferred_rnti_type);
    srsran_assert(not ss_candidate_list.empty(), "No searchSpace candidates for rnti type={}", preferred_rnti_type);
  }

  // Check if a candidate has valid parameters for an allocation.
  bool is_candidate_valid(const candidate& current) const
  {
    if (ue_cc.cfg().cell_cfg_common.get_nof_dl_symbol_per_slot(pdcch_slot + current.pdsch_td_res().k0) <
        current.pdsch_td_res().symbols.stop()) {
      // Check whether PDSCH time domain resource fits in DL symbols of the slot.
      return false;
    }

    // Check whether PDSCH time domain resource does not overlap with CORESET.
    if (current.pdsch_td_res().symbols.start() <
        current.ss().cfg->get_first_symbol_index() + current.ss().coreset->duration) {
      return false;
    }

    // If it is a retx, we need to ensure we use a time_domain_resource with the same number of symbols as used for
    // the first transmission.
    if (is_retx and current.pdsch_td_res().symbols.length() != current.harq().last_alloc_params().nof_symbols) {
      return false;
    }

    return true;
  }

  // Iterate over the list of candidates until a valid one is found.
  void iterate_until_valid_candidate_found(candidate& current)
  {
    for (; current.harq_it != dl_harq_candidates.end(); ++current.harq_it) {
      // If the HARQ candidate changed, generate new list of Search Spaces.
      generate_ss_candidates(current.harq_it);

      for (; current.ss_it != ss_candidate_list.end(); ++current.ss_it) {
        if ((*current.ss_it)->cfg->is_search_space0()) {
          // Skip SearchSpace#0.
          continue;
        }

        for (; current.time_res < (*current.ss_it)->pdsch_time_domain_list.size(); ++current.time_res) {
          if (is_candidate_valid(current)) {
            // Valid candidate found.
            return;
          }
        }
        current.time_res = 0;
      }
      current.ss_it = ss_candidate_list.begin();
    }

    // Iteration finished.
    ss_candidate_list.clear();
    harq_of_ss_list  = nullptr;
    current.ss_it    = nullptr;
    current.time_res = 0;
  }

  const ue&      ue_ref;
  const ue_cell& ue_cc;
  const bool     is_retx;

  static_vector<const dl_harq_process*, MAX_NOF_HARQS> dl_harq_candidates;

  search_space_candidate_list ss_candidate_list;
  const dl_harq_process*      harq_of_ss_list = nullptr;
  slot_point                  pdcch_slot;
};

} // namespace srsran