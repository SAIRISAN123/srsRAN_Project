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

#include "srsran/phy/metrics/phy_metrics_notifiers.h"
#include "srsran/phy/metrics/phy_metrics_reports.h"
#include "srsran/phy/upper/channel_coding/crc_calculator.h"
#include <memory>

namespace srsran {

/// CRC calculator metric decorator.
class phy_metrics_crc_calculator_decorator : public crc_calculator
{
public:
  /// Creates an CRC calculator decorator from a base instance and a metric notifier.
  phy_metrics_crc_calculator_decorator(std::unique_ptr<crc_calculator> base_calculator_,
                                       crc_calculator_metric_notifier& notifier_) :
    base_calculator(std::move(base_calculator_)), notifier(notifier_)
  {
    srsran_assert(base_calculator, "Invalid CRC calculator.");
  }

  // See interface for documentation.
  crc_calculator_checksum_t calculate_byte(span<const uint8_t> data) const override
  {
    auto                      tp_before = std::chrono::high_resolution_clock::now();
    crc_calculator_checksum_t ret       = base_calculator->calculate_byte(data);
    auto                      tp_after  = std::chrono::high_resolution_clock::now();

    notifier.new_metric({.poly     = base_calculator->get_generator_poly(),
                         .nof_bits = units::bytes(data.size()).to_bits(),
                         .elapsed  = tp_after - tp_before});

    return ret;
  }

  // See interface for documentation.
  crc_calculator_checksum_t calculate_bit(span<const uint8_t> data) const override
  {
    auto                      tp_before = std::chrono::high_resolution_clock::now();
    crc_calculator_checksum_t ret       = base_calculator->calculate_bit(data);
    auto                      tp_after  = std::chrono::high_resolution_clock::now();

    // Create report metrics.
    notifier.new_metric({.poly     = base_calculator->get_generator_poly(),
                         .nof_bits = units::bits(data.size()),
                         .elapsed  = tp_after - tp_before});

    return ret;
  }

  // See interface for documentation.
  crc_calculator_checksum_t calculate(const bit_buffer& data) const override
  {
    auto                      tp_before = std::chrono::high_resolution_clock::now();
    crc_calculator_checksum_t ret       = base_calculator->calculate(data);
    auto                      tp_after  = std::chrono::high_resolution_clock::now();

    // Create report metrics.
    notifier.new_metric({.poly     = base_calculator->get_generator_poly(),
                         .nof_bits = units::bits(data.size()),
                         .elapsed  = tp_after - tp_before});

    return ret;
  }

  // See interface for documentation.
  crc_generator_poly get_generator_poly() const override { return base_calculator->get_generator_poly(); }

private:
  std::unique_ptr<crc_calculator> base_calculator;
  crc_calculator_metric_notifier& notifier;
};

} // namespace srsran
