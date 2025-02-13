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

#include "apps/services/network/udp_cli11_schema.h"
#include "apps/units/o_cu_up/cu_up/cu_up_unit_pcap_config.h"
#include "cu_up_unit_logger_config.h"
#include "srsran/ran/gnb_cu_up_id.h"
#include "srsran/ran/gnb_id.h"
#include "srsran/ran/qos/five_qi.h"

namespace srsran {

/// Metrics configuration.
struct cu_up_unit_metrics_config {
  /// Statistics report period in seconds.
  unsigned cu_up_statistics_report_period = 1;
  struct pdcp_metrics {
    unsigned report_period = 0; // PDCP report period in ms
  } pdcp;
  bool enable_json_metrics = false;
};

struct cu_up_unit_ngu_socket_config {
  std::string   bind_addr      = "auto";
  std::string   bind_interface = "auto";
  std::string   ext_addr       = "auto";
  udp_appconfig udp_config     = {};
};

struct cu_up_unit_ngu_config {
  bool                                      no_core = false;
  std::vector<cu_up_unit_ngu_socket_config> ngu_socket_cfg;
};

/// F1-U configuration at CU_UP side
struct cu_cp_unit_f1u_config {
  int32_t t_notify; ///< Maximum backoff time for discard notifications from CU_UP to DU (ms)
};

/// QoS configuration.
struct cu_up_unit_qos_config {
  five_qi_t             five_qi = uint_to_five_qi(9);
  std::string           mode    = "am";
  cu_cp_unit_f1u_config f1u_cu_up;
};

struct cu_up_unit_test_mode_config {
  bool     enabled           = false;
  bool     integrity_enabled = true;
  bool     ciphering_enabled = true;
  uint16_t nea_algo          = 2;
  uint16_t nia_algo          = 2;
};

/// CU-UP application unit configuration.
struct cu_up_unit_config {
  /// gNB identifier.
  gnb_id_t gnb_id = {411, 22};
  /// CU-UP identifier.
  gnb_cu_up_id_t gnb_cu_up_id = gnb_cu_up_id_t::min;
  /// GPTU parameters.
  unsigned gtpu_queue_size          = 2048;
  unsigned gtpu_reordering_timer_ms = 0;
  bool     warn_on_drop             = false;
  /// UPF configuration.
  cu_up_unit_ngu_config ngu_cfg;
  /// Metrics.
  cu_up_unit_metrics_config metrics;
  /// Loggers.
  cu_up_unit_logger_config loggers;
  /// PCAPs.
  cu_up_unit_pcap_config pcap_cfg;
  /// QoS configuration.
  std::vector<cu_up_unit_qos_config> qos_cfg;
  /// Test mode.
  cu_up_unit_test_mode_config test_mode_cfg;
};

} // namespace srsran
