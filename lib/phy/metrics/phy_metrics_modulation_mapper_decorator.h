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
#include "srsran/phy/upper/channel_modulation/modulation_mapper.h"
#include "srsran/phy/upper/unique_rx_buffer.h"
#include "srsran/support/resource_usage/scoped_resource_usage.h"

namespace srsran {

/// Modulation mapper metric decorator.
class phy_metrics_modulation_mapper_decorator : public modulation_mapper
{
public:
  /// Creates a modulation mapper decorator from a base instance and a metric notifier.
  phy_metrics_modulation_mapper_decorator(std::unique_ptr<modulation_mapper>         base_,
                                          common_channel_modulation_metric_notifier& notifier_) :
    base(std::move(base_)), notifier(notifier_)
  {
    srsran_assert(base, "Invalid base instance.");
  }

  // See interface for documentation.
  void modulate(span<cf_t> symbols, const bit_buffer& input, modulation_scheme scheme) override
  {
    channel_modulation_metrics metrics;
    {
      // Use scoped resource usage class to measure CPU usage of this block.
      resource_usage_utils::scoped_resource_usage rusage_tracker(metrics.measurements);
      base->modulate(symbols, input, scheme);
    }
    metrics.modulation  = scheme;
    metrics.nof_symbols = static_cast<unsigned>(symbols.size());
    notifier.on_new_metric(metrics);
  }

  // See interface for documentation.
  float modulate(span<ci8_t> symbols, const bit_buffer& input, modulation_scheme scheme) override
  {
    channel_modulation_metrics metrics;
    float                      ret;
    {
      // Use scoped resource usage class to measure CPU usage of this block.
      resource_usage_utils::scoped_resource_usage rusage_tracker(metrics.measurements);
      ret = base->modulate(symbols, input, scheme);
    }
    metrics.modulation  = scheme;
    metrics.nof_symbols = static_cast<unsigned>(symbols.size());
    notifier.on_new_metric(metrics);

    return ret;
  }

private:
  std::unique_ptr<modulation_mapper>         base;
  common_channel_modulation_metric_notifier& notifier;
};

} // namespace srsran
