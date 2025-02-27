/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#pragma once

#include "../policy/scheduler_policy.h"
#include "ue_cell_grid_allocator.h"

namespace srsran {

class pdcch_resource_allocator;
class uci_allocator;
class scheduler_policy;

class intra_slice_scheduler
{
public:
  intra_slice_scheduler(const scheduler_ue_expert_config& expert_cfg_,
                        ue_repository&                    ues_,
                        srslog::basic_logger&             logger_);

  void add_cell(du_cell_index_t           cell_index,
                pdcch_resource_allocator& pdcch_sched,
                uci_allocator&            uci_alloc,
                cell_resource_allocator&  cell_alloc,
                cell_harq_manager&        cell_harqs);

  /// Reset context in preparation for new slot.
  void slot_indication(slot_point sl_tx);

  /// Called once all the UE grants have been allocated for a slot and cell.
  void post_process_results();

  /// Schedule DL grants for a given slice candidate.
  void dl_sched(slot_point             pdcch_slot,
                du_cell_index_t        cell_index,
                dl_ran_slice_candidate slice,
                scheduler_policy&      dl_policy);

  /// Schedule UL grants for a given slice candidate.
  void ul_sched(slot_point             pdcch_slot,
                du_cell_index_t        cell_index,
                ul_ran_slice_candidate slice,
                scheduler_policy&      dl_policy);

private:
  /// Context of a given cell.
  struct cell_t {
    du_cell_index_t           cell_index;
    pdcch_resource_allocator* pdcch_sched;
    uci_allocator*            uci_alloc;
    cell_resource_allocator*  cell_alloc;
    cell_harq_manager*        cell_harqs;
  };

  /// Determines whether a UE can be DL scheduled in a given slot.
  bool can_allocate_pdsch(slot_point      sl_tx,
                          slot_point      sl_pdsch,
                          du_cell_index_t cell_index,
                          const slice_ue& u,
                          const ue_cell&  ue_cc) const;

  /// Determines whether a UE can be UL scheduled in a given slot.
  bool can_allocate_pusch(slot_point      pdcch_slot,
                          slot_point      pusch_slot,
                          du_cell_index_t cell_index,
                          const slice_ue& u,
                          const ue_cell&  ue_cc) const;

  std::optional<ue_newtx_candidate> create_newtx_dl_candidate(slot_point      pdcch_slot,
                                                              slot_point      pdsch_slot,
                                                              du_cell_index_t cell_index,
                                                              const slice_ue& u) const;

  std::optional<ue_newtx_candidate> create_newtx_ul_candidate(slot_point      pdcch_slot,
                                                              slot_point      pusch_slot,
                                                              du_cell_index_t cell_index,
                                                              const slice_ue& u) const;

  void prepare_newtx_dl_candidates(du_cell_index_t               cell_index,
                                   const dl_ran_slice_candidate& slice,
                                   scheduler_policy&             dl_policy);

  void prepare_newtx_ul_candidates(du_cell_index_t               cell_index,
                                   const ul_ran_slice_candidate& slice,
                                   scheduler_policy&             dl_policy);

  unsigned schedule_dl_retx_candidates(du_cell_index_t               cell_index,
                                       const dl_ran_slice_candidate& slice,
                                       unsigned                      max_ue_grants_to_alloc);

  unsigned schedule_ul_retx_candidates(du_cell_index_t               cell_index,
                                       const ul_ran_slice_candidate& slice,
                                       unsigned                      max_ue_grants_to_alloc);

  unsigned schedule_dl_newtx_candidates(du_cell_index_t         cell_index,
                                        dl_ran_slice_candidate& slice,
                                        scheduler_policy&       dl_policy,
                                        unsigned                max_ue_grants_to_alloc);

  unsigned schedule_ul_newtx_candidates(du_cell_index_t         cell_index,
                                        ul_ran_slice_candidate& slice,
                                        scheduler_policy&       ul_policy,
                                        unsigned                max_ue_grants_to_alloc);

  unsigned max_pdschs_to_alloc(slot_point pdcch_slot, const dl_ran_slice_candidate& slice, du_cell_index_t cell_index);

  unsigned max_puschs_to_alloc(slot_point pdcch_slot, const ul_ran_slice_candidate& slice, du_cell_index_t cell_index);

  const scheduler_ue_expert_config& expert_cfg;
  srslog::basic_logger&             logger;

  slotted_id_vector<du_cell_index_t, cell_t> cells;

  ue_cell_grid_allocator ue_alloc;

  slot_point last_sl_tx;

  // Number of allocation attempts for DL in the given slot.
  unsigned dl_attempts_count = 0;
  unsigned ul_attempts_count = 0;

  std::vector<ue_newtx_candidate> dl_newtx_candidates;
  std::vector<ue_newtx_candidate> ul_newtx_candidates;
};

} // namespace srsran
