/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "ue_context_release_procedure.h"
#include "../f1ap_asn1_converters.h"
#include "srsran/f1ap/common/f1ap_message.h"
#include "srsran/ran/lcid.h"
#include "srsran/support/srsran_assert.h"

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::f1ap;

ue_context_release_procedure::ue_context_release_procedure(const f1ap_ue_context_release_command& cmd_,
                                                           f1ap_ue_context&                       ue_ctxt_,
                                                           f1ap_message_notifier&                 f1ap_notif_,
                                                           std::chrono::milliseconds              proc_timeout_) :
  ue_ctxt(ue_ctxt_),
  f1ap_notifier(f1ap_notif_),
  proc_timeout(proc_timeout_),
  logger(srslog::fetch_basic_logger("CU-CP-F1"))
{
  command->gnb_cu_ue_f1ap_id = gnb_cu_ue_f1ap_id_to_uint(ue_ctxt.ue_ids.cu_ue_f1ap_id);
  command->gnb_du_ue_f1ap_id = gnb_du_ue_f1ap_id_to_uint(ue_ctxt.ue_ids.du_ue_f1ap_id);
  command->cause             = cause_to_asn1(cmd_.cause);
  if (!cmd_.rrc_release_pdu.empty()) {
    command->rrc_container_present = true;
    command->rrc_container         = cmd_.rrc_release_pdu.copy();

    srsran_assert(cmd_.srb_id.has_value(), "SRB-ID for UE Context Release Command with RRC Container must be set");

    command->srb_id_present = true;
    command->srb_id         = srb_id_to_uint(cmd_.srb_id.value());
  }
}

void ue_context_release_procedure::operator()(coro_context<async_task<ue_index_t>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("{}: Procedure started...", f1ap_ue_log_prefix{ue_ctxt.ue_ids, name()});

  transaction_sink.subscribe_to(ue_ctxt.ev_mng.context_release_complete, proc_timeout);

  ue_ctxt.marked_for_release = true;

  // Send command to DU.
  send_ue_context_release_command();

  // Await CU response.
  CORO_AWAIT(transaction_sink);

  // Handle response from DU and return UE index
  CORO_RETURN(create_ue_context_release_complete(
      transaction_sink.successful() ? transaction_sink.response() : asn1::f1ap::ue_context_release_complete_s{}));
}

void ue_context_release_procedure::send_ue_context_release_command()
{
  // Pack message into PDU
  f1ap_message f1ap_ue_ctxt_rel_msg;
  f1ap_ue_ctxt_rel_msg.pdu.set_init_msg();
  f1ap_ue_ctxt_rel_msg.pdu.init_msg().load_info_obj(ASN1_F1AP_ID_UE_CONTEXT_RELEASE);
  f1ap_ue_ctxt_rel_msg.pdu.init_msg().value.ue_context_release_cmd() = command;

  // send UE Context Release Command
  f1ap_notifier.on_new_message(f1ap_ue_ctxt_rel_msg);
}

ue_index_t
ue_context_release_procedure::create_ue_context_release_complete(const asn1::f1ap::ue_context_release_complete_s& msg)
{
  ue_index_t ret = ue_index_t::invalid;

  if (msg->gnb_du_ue_f1ap_id == gnb_du_ue_f1ap_id_to_uint(ue_ctxt.ue_ids.du_ue_f1ap_id)) {
    ret = ue_ctxt.ue_ids.ue_index;
    logger.info("{}: Procedure finished successfully.", f1ap_ue_log_prefix{ue_ctxt.ue_ids, name()});
  } else {
    logger.warning("{}: Procedure failed.", f1ap_ue_log_prefix{ue_ctxt.ue_ids, name()});
  }

  return ret;
}
