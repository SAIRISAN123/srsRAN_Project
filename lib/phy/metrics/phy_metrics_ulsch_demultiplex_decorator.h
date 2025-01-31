/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/phy/upper/channel_processors/pusch/pusch_codeword_buffer.h"
#include "srsran/phy/upper/channel_processors/pusch/ulsch_demultiplex.h"

namespace srsran {

/// UL-SCH demultiplexer metric decorator.
class phy_metrics_ulsch_demultiplex_decorator : public ulsch_demultiplex, private pusch_codeword_buffer
{
public:
  /// Creates a UL-SCH demultiplexer decorator from a base instance and a notifier.
  phy_metrics_ulsch_demultiplex_decorator(std::unique_ptr<ulsch_demultiplex> base_,
                                          ulsch_demultiplex_metric_notifier& notifier_) :
    base(std::move(base_)), notifier(notifier_)
  {
    srsran_assert(base, "Invalid UL-SCH demultiplexer.");
  }

  // See ulsch_demultiplex interface for documentation.
  void set_csi_part2(pusch_decoder_buffer& csi_part2,
                     unsigned int          nof_csi_part2_bits,
                     unsigned int          nof_csi_part2_enc_bits) override
  {
    base->set_csi_part2(csi_part2, nof_csi_part2_bits, nof_csi_part2_enc_bits);
  }

  // See ulsch_demultiplex interface for documentation.
  pusch_codeword_buffer& demultiplex(pusch_decoder_buffer& sch_data,
                                     pusch_decoder_buffer& harq_ack,
                                     pusch_decoder_buffer& csi_part1,
                                     const configuration&  config) override
  {
    srsran_assert(base_buffer == nullptr, "Invalid base buffer.");

    // Prepare base and save the base buffer.
    auto tp_before = std::chrono::high_resolution_clock::now();
    base_buffer    = &base->demultiplex(sch_data, harq_ack, csi_part1, config);
    auto tp_after  = std::chrono::high_resolution_clock::now();

    // Save the initial elapsed time.
    elapsed_init = tp_after - tp_before;

    return *this;
  }

private:
  // See pusch_codeword_buffer interface for documentation.
  span<log_likelihood_ratio> get_next_block_view(unsigned block_size) override
  {
    srsran_assert(base_buffer != nullptr, "Invalid base buffer.");

    // Assume this method takes a small amount of time.
    return base_buffer->get_next_block_view(block_size);
  }

  // See pusch_codeword_buffer interface for documentation.
  void on_new_block(span<const log_likelihood_ratio> data, const bit_buffer& scrambling_seq) override
  {
    srsran_assert(base_buffer != nullptr, "Invalid base buffer.");

    auto tp_before = std::chrono::high_resolution_clock::now();
    base_buffer->on_new_block(data, scrambling_seq);
    auto tp_after = std::chrono::high_resolution_clock::now();

    // Accumulate elapsed time and number of bits.
    elapsed_on_new_block += tp_after - tp_before;
    sum_nof_bits += data.size();
  }

  // See pusch_codeword_buffer interface for documentation.
  void on_end_codeword() override
  {
    srsran_assert(base_buffer != nullptr, "Invalid base buffer.");

    auto tp_before = std::chrono::high_resolution_clock::now();
    base_buffer->on_end_codeword();
    auto tp_after = std::chrono::high_resolution_clock::now();

    // Notify metrics.
    notifier.new_metric({.elapsed_init            = elapsed_init,
                         .elapsed_on_new_block    = elapsed_on_new_block,
                         .elapsed_on_end_codeword = tp_after - tp_before,
                         .sum_nof_bits            = sum_nof_bits});

    // Reset base buffer pointer and counters
    base_buffer          = nullptr;
    elapsed_init         = {};
    elapsed_on_new_block = {};
    sum_nof_bits         = 0;
  }

  std::unique_ptr<ulsch_demultiplex> base;
  ulsch_demultiplex_metric_notifier& notifier;
  pusch_codeword_buffer*             base_buffer          = nullptr;
  std::chrono::nanoseconds           elapsed_init         = {};
  std::chrono::nanoseconds           elapsed_on_new_block = {};
  unsigned                           sum_nof_bits         = 0;
};

} // namespace srsran
