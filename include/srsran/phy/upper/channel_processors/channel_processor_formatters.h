/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/phy/support/precoding_formatters.h"
#include "srsran/phy/support/re_pattern_formatters.h"
#include "srsran/phy/upper/channel_processors/pdcch/pdcch_processor.h"
#include "srsran/phy/upper/channel_processors/prach_detector.h"
#include "srsran/phy/upper/channel_processors/ssb_processor.h"
#include "srsran/phy/upper/channel_state_information_formatters.h"
#include "srsran/srsvec/copy.h"

namespace fmt {

/// \brief Custom formatter for \c prach_detector::configuration.
template <>
struct formatter<srsran::prach_detector::configuration> {
  /// Helper used to parse formatting options and format fields.
  srsran::delimited_formatter helper;

  /// Default constructor.
  formatter() = default;

  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return helper.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const srsran::prach_detector::configuration& config, FormatContext& ctx) const
  {
    helper.format_always(ctx, "rsi={}", config.root_sequence_index);
    helper.format_if_verbose(ctx,
                             "preambles=[{}, {})",
                             config.start_preamble_index,
                             config.start_preamble_index + config.nof_preamble_indices);
    helper.format_if_verbose(ctx, "format={}", to_string(config.format));
    helper.format_if_verbose(ctx, "set={}", to_string(config.restricted_set));
    helper.format_if_verbose(ctx, "zcz={}", config.zero_correlation_zone);
    helper.format_if_verbose(ctx, "scs={}", to_string(config.ra_scs));
    helper.format_if_verbose(ctx, "nof_rx_ports={}", config.nof_rx_ports);

    return ctx.out();
  }
};

/// \brief Custom formatter for \c prach_detection_result::preamble_indication.
template <>
struct formatter<srsran::prach_detection_result::preamble_indication> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const srsran::prach_detection_result::preamble_indication& preamble, FormatContext& ctx) const
  {
    format_to(ctx.out(),
              "{{idx={} ta={:.2f}us detection_metric={:.1f}}}",
              preamble.preamble_index,
              preamble.time_advance.to_seconds() * 1e6,
              preamble.detection_metric);
    return ctx.out();
  }
};

/// \brief Custom formatter for \c prach_detection_result.
template <>
struct formatter<srsran::prach_detection_result> {
  /// Helper used to parse formatting options and format fields.
  srsran::delimited_formatter helper;

  /// Default constructor.
  formatter() = default;

  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return helper.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const srsran::prach_detection_result& result, FormatContext& ctx) const
  {
    helper.format_always(ctx, "rssi={:+.1f}dB", result.rssi_dB);
    helper.format_if_verbose(ctx, "res={:.1f}us", result.time_resolution.to_seconds() * 1e6);
    helper.format_if_verbose(ctx, "max_ta={:.2f}us", result.time_advance_max.to_seconds() * 1e6);
    helper.format_always(ctx,
                         "detected_preambles=[{:,}]",
                         srsran::span<const srsran::prach_detection_result::preamble_indication>(result.preambles));

    return ctx.out();
  }
};

/// \brief Custom formatter for \c ssb_processor::pdu_t.
template <>
struct formatter<srsran::ssb_processor::pdu_t> {
  /// Helper used to parse formatting options and format fields.
  srsran::delimited_formatter helper;

  /// Default constructor.
  formatter() = default;

  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return helper.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const srsran::ssb_processor::pdu_t& pdu, FormatContext& ctx) const
  {
    helper.format_always(ctx, "pci={}", pdu.phys_cell_id);
    helper.format_always(ctx, "ssb_idx={}", pdu.ssb_idx);
    helper.format_always(ctx, "L_max={}", pdu.L_max);
    helper.format_always(ctx, "common_scs={}", scs_to_khz(pdu.common_scs));
    helper.format_always(ctx, "sc_offset={}", pdu.subcarrier_offset.value());
    helper.format_always(ctx, "offset_PointA={}", pdu.offset_to_pointA.value());
    helper.format_always(ctx, "pattern={}", to_string(pdu.pattern_case));

    helper.format_if_verbose(ctx, "beta_pss={:+.1f}dB", pdu.beta_pss);
    helper.format_if_verbose(ctx, "slot={}", pdu.slot);
    helper.format_if_verbose(ctx, "ports={}", srsran::span<const uint8_t>(pdu.ports));

    return ctx.out();
  }
};

} // namespace fmt
