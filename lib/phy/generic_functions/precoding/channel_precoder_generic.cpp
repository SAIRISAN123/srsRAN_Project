/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "channel_precoder_generic.h"

using namespace srsran;

void channel_precoder_generic::apply_precoding_port(span<cf_t>                port_re,
                                                    const re_buffer_reader<>& input_re,
                                                    span<const cf_t>          port_weights) const
{
  unsigned nof_re     = input_re.get_nof_re();
  unsigned nof_layers = input_re.get_nof_slices();

  std::array<span<const cf_t>, precoding_constants::MAX_NOF_LAYERS> layer_re_view_list;
  for (unsigned i_layer = 0; i_layer != nof_layers; ++i_layer) {
    layer_re_view_list[i_layer] = input_re.get_slice(i_layer);
  }

  for (unsigned i_re = 0; i_re != nof_re; ++i_re) {
    // Set the port RE to the contribution of the first layer.
    port_re[i_re] = layer_re_view_list[0][i_re] * port_weights[0];

    for (unsigned i_layer = 1; i_layer != nof_layers; ++i_layer) {
      // Accumulate the contributions of all other layers.
      port_re[i_re] += layer_re_view_list[i_layer][i_re] * port_weights[i_layer];
    }
  }
}

void channel_precoder_generic::apply_layer_map_and_precoding(re_buffer_writer<>&            output,
                                                             span<const ci8_t>              input,
                                                             const precoding_weight_matrix& precoding) const
{
  unsigned nof_re     = output.get_nof_re();
  unsigned nof_layers = precoding.get_nof_layers();
  unsigned nof_ports  = precoding.get_nof_ports();

  for (unsigned i_re = 0; i_re != nof_re; ++i_re) {
    for (unsigned i_port = 0; i_port != nof_ports; ++i_port) {
      span<const cf_t> port_weights = precoding.get_port_coefficients(i_port);
      span<cf_t>       port_re      = output.get_slice(i_port);

      cf_t sum = to_cf(input[nof_layers * i_re]) * port_weights[0];
      for (unsigned i_layer = 1; i_layer != nof_layers; ++i_layer) {
        sum += to_cf(input[nof_layers * i_re + i_layer]) * port_weights[i_layer];
      }
      port_re[i_re] = sum;
    }
  }
}
