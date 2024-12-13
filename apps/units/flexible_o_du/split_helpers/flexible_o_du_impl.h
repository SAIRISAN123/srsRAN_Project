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

#include "srsran/du/du.h"
#include "srsran/du/du_power_controller.h"
#include "srsran/du/o_du.h"
#include "srsran/ru/ru_adapters.h"
#include <memory>

namespace srsran {

class radio_unit;

/// \brief Flexible O-RAN DU implementation.
///
/// One O-RAN DU can handle more than one cell.
class flexible_o_du_impl : public srs_du::du, public du_power_controller
{
public:
  explicit flexible_o_du_impl(unsigned nof_cells_);

  // See interface for documentation.
  du_power_controller& get_power_controller() override { return *this; }

  // See interface for documentation.
  void start() override;

  // See interface for documentation.
  void stop() override;

  /// Adds the given RU to this flexible O-RAN DU.
  void add_ru(std::unique_ptr<radio_unit> active_ru);

  /// Adds the given DU to this flexible O-RAN DU.
  void add_du(std::unique_ptr<srs_du::o_du> active_du);

  /// Getters to the adaptors.
  upper_phy_ru_ul_adapter&         get_upper_ru_ul_adapter() { return ru_ul_adapt; }
  upper_phy_ru_timing_adapter&     get_upper_ru_timing_adapter() { return ru_timing_adapt; }
  upper_phy_ru_error_adapter&      get_upper_ru_error_adapter() { return ru_error_adapt; }
  upper_phy_ru_dl_rg_adapter&      get_upper_ru_dl_rg_adapter() { return ru_dl_rg_adapt; }
  upper_phy_ru_ul_request_adapter& get_upper_ru_ul_request_adapter() { return ru_ul_request_adapt; }

private:
  const unsigned                  nof_cells;
  upper_phy_ru_ul_adapter         ru_ul_adapt;
  upper_phy_ru_timing_adapter     ru_timing_adapt;
  upper_phy_ru_error_adapter      ru_error_adapt;
  std::unique_ptr<srs_du::o_du>   du;
  std::unique_ptr<radio_unit>     ru;
  upper_phy_ru_dl_rg_adapter      ru_dl_rg_adapt;
  upper_phy_ru_ul_request_adapter ru_ul_request_adapt;
};

} // namespace srsran
