/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "f1u_cli11_schema.h"
#include "srsran/support/cli11_utils.h"
#include "srsran/support/config_parsers.h"

using namespace srsran;

static void configure_cli11_f1u_socket_args(CLI::App& app, f1u_socket_appconfig& f1u_cfg)
{
  add_option(app,
             "--bind_addr",
             f1u_cfg.bind_addr,
             "Default local IP address interfaces bind to, unless a specific bind address is specified")
      ->check(CLI::ValidIPV4);
  add_option(app, "--five_qi", f1u_cfg.five_qi, "Assign this socket to a specific 5QI")->check(CLI::Range(0, 255));

  configure_cli11_with_udp_config_schema(app, f1u_cfg.udp_config);
}

void srsran::configure_cli11_f1u_sockets_args(CLI::App& app, f1u_sockets_appconfig& f1u_params)
{
  // Add option for multiple sockets, for usage with different slices, 5QIs or parallization.
  auto sock_lambda = [&f1u_params](const std::vector<std::string>& values) {
    // Prepare the radio bearers
    f1u_params.f1u_socket_cfg.resize(values.size());

    // Format every F1-U socket configuration.
    for (unsigned i = 0, e = values.size(); i != e; ++i) {
      CLI::App subapp("F1-U socket parameters", "F1-U socket config, item #" + std::to_string(i));
      subapp.config_formatter(create_yaml_config_parser());
      subapp.allow_config_extras(CLI::config_extras_mode::capture);
      configure_cli11_f1u_socket_args(subapp, f1u_params.f1u_socket_cfg[i]);
      std::istringstream ss(values[i]);
      subapp.parse_from_stream(ss);
    }
  };
  add_option_cell(app, "--socket", sock_lambda, "Configures UDP/IP socket parameters of the F1-U interface");
}
