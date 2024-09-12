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

namespace srsran {

class du_power_controller;

namespace srs_du {

/// Public DU interface.
class du
{
public:
  virtual ~du() = default;

  /// Returns the power controller of this DU.
  virtual du_power_controller& get_power_controller() = 0;
};

} // namespace srs_du
} // namespace srsran
