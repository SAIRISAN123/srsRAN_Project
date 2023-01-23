/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "initial_du_setup_procedure.h"
#include "../converters/f1c_configuration_helpers.h"
#include "../converters/mac_config_helpers.h"
#include "../converters/scheduler_configuration_helpers.h"
#include "../du_cell_manager.h"
#include "srsgnb/asn1/f1ap/f1ap.h"
#include "srsgnb/scheduler/config/scheduler_cell_config_validator.h"

using namespace srsgnb;
using namespace srs_du;

initial_du_setup_procedure::initial_du_setup_procedure(const du_manager_params& params_, du_cell_manager& cell_mng_) :
  params(params_), cell_mng(cell_mng_), logger(srslog::fetch_basic_logger("DU-MNG"))
{
}

void initial_du_setup_procedure::operator()(coro_context<async_task<void>>& ctx)
{
  CORO_BEGIN(ctx);

  // Initiate F1 Setup.
  CORO_AWAIT_VALUE(response_msg, start_f1_setup_request());

  // Handle F1 setup result.
  if (response_msg.success) {
    // In case of F1 Setup Response.
    handle_f1_setup_response(response_msg.msg);
  }

  // Configure DU Cells.
  for (unsigned idx = 0; idx < cell_mng.nof_cells(); ++idx) {
    du_cell_index_t         cell_index   = to_du_cell_index(idx);
    const du_cell_config&   du_cfg       = cell_mng.get_cell_cfg(cell_index);
    byte_buffer             sib1_payload = srs_du::make_asn1_rrc_cell_bcch_dl_sch_msg(du_cfg);
    auto                    sched_cfg = srs_du::make_sched_cell_config_req(cell_index, du_cfg, sib1_payload.length());
    error_type<std::string> result    = config_validators::validate_sched_cell_configuration_request_message(sched_cfg);
    if (result.is_error()) {
      report_fatal_error("Invalid cell={} configuration. Cause: {}", cell_index, result.error());
    }
    params.mac.cell_mng.add_cell(make_mac_cell_config(cell_index, du_cfg, std::move(sib1_payload), sched_cfg));
  }

  // Activate DU Cells.
  params.mac.cell_mng.get_cell_controller(to_du_cell_index(0)).start();

  CORO_RETURN();
}

async_task<f1_setup_response_message> initial_du_setup_procedure::start_f1_setup_request()
{
  // Prepare request to send to F1.
  std::vector<const du_cell_config*> cells_to_add;
  for (size_t i = 0; i < cell_mng.nof_cells(); ++i) {
    cells_to_add.push_back(&cell_mng.get_cell_cfg(to_du_cell_index(i)));
  }
  du_setup_params setup_params;
  setup_params.gnb_du_id               = params.ran.gnb_du_id;
  setup_params.rrc_version             = params.ran.rrc_version;
  setup_params.gnb_du_id               = params.ran.gnb_du_id;
  f1_setup_request_message request_msg = {};
  fill_asn1_f1_setup_request(request_msg.msg, setup_params, cells_to_add);

  // Initiate F1 Setup Request.
  return params.f1ap.conn_mng.handle_f1ap_setup_request(request_msg);
}

void initial_du_setup_procedure::handle_f1_setup_response(const asn1::f1ap::f1_setup_resp_s& resp)
{
  // TODO

  if (resp->cells_to_be_activ_list_present) {
    for (const auto& cell : resp->cells_to_be_activ_list.value) {
      const asn1::f1ap::cells_to_be_activ_list_item_s& item = cell->cells_to_be_activ_list_item();
      (void)item;
      // TODO: Handle cells to activate.
    }
  }
}
