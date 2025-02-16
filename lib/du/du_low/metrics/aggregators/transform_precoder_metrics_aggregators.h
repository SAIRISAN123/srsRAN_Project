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
#include <atomic>

namespace srsran {

/// Transform precoder metrics aggregator.
class transform_precoder_metrics_aggregator : public transform_precoder_metric_notifier
{
public:
  /// Gets the transform precoder metric interface.
  transform_precoder_metric_notifier& get_notifier() { return *this; }

  /// Gets the average processing latency in microseconds.
  double get_avg_latency_us() const
  {
    return count.load(std::memory_order_relaxed)
               ? static_cast<double>(sum_elapsed_ns) / static_cast<double>(count) * 1e-3
               : 0;
  }

  /// Gets the average processing rate in MREps (millions of resource elements per second).
  double get_avg_rate_MREps() const
  {
    return sum_elapsed_ns.load(std::memory_order_relaxed)
               ? static_cast<double>(sum_nof_re) / static_cast<double>(sum_elapsed_ns) * 1000
               : 0;
  }

  /// Gets the total amount of time the transform precoding spent calculating.
  std::chrono::nanoseconds get_total_time() const { return std::chrono::nanoseconds(sum_elapsed_ns); }

  /// Gets the CPU usage in microseconds of the Transform precoder.
  double get_cpu_usage_us() const { return static_cast<double>(sum_elapsed_ns) / 1000.0; }

  /// Resets values of all internal counters.
  void reset()
  {
    count          = 0;
    sum_nof_re     = 0;
    sum_elapsed_ns = 0;
  }

private:
  // See interface for documentation.
  void on_new_metric(const transform_precoder_metrics& metrics) override
  {
    sum_nof_re += metrics.nof_re;
    sum_elapsed_ns += metrics.measurements.duration.count();
    ++count;
  }

  std::atomic<uint64_t> sum_nof_re     = {};
  std::atomic<uint64_t> sum_elapsed_ns = {};
  std::atomic<uint64_t> count          = {};
};

} // namespace srsran
