/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsgnb/cu_cp/cu_cp.h"
#include "srsgnb/cu_cp/cu_cp_types.h"
#include "srsgnb/cu_cp/ue_context.h"
#include "srsgnb/support/async/async_task_loop.h"

namespace srsgnb {
namespace srs_cu_cp {

struct dummy_du_processor_to_cu_cp_task_scheduler : public du_processor_to_cu_cp_task_scheduler {
public:
  dummy_du_processor_to_cu_cp_task_scheduler(timer_manager& timers_) : timer_db(timers_) {}
  void schedule_async_task(du_index_t du_index, ue_index_t ue_index, async_task<void>&& task) override
  {
    ctrl_loop.schedule(std::move(task));
  }
  unique_timer   make_unique_timer() override { return timer_db.create_unique_timer(); }
  timer_manager& get_timer_manager() override { return timer_db; }

  void tick_timer() { timer_db.tick_all(); }

private:
  async_task_sequencer ctrl_loop{16};
  timer_manager&       timer_db;
};

struct dummy_du_processor_cu_cp_notifier : public du_processor_cu_cp_notifier {
public:
  dummy_du_processor_cu_cp_notifier(cu_cp_du_handler* cu_cp_handler_) :
    logger(srslog::fetch_basic_logger("TEST")), cu_cp_handler(cu_cp_handler_)
  {
  }

  void attach_handler(cu_cp_du_handler* cu_cp_handler_) { cu_cp_handler = cu_cp_handler_; }

  void on_rrc_ue_created(du_index_t du_index, ue_index_t ue_index, rrc_ue_interface* rrc_ue) override
  {
    logger.info("Received a RRC UE creation notification");

    if (cu_cp_handler != nullptr) {
      cu_cp_handler->handle_rrc_ue_creation(du_index, ue_index, rrc_ue);
    }
  }

private:
  srslog::basic_logger& logger;
  cu_cp_du_handler*     cu_cp_handler = nullptr;
};

} // namespace srs_cu_cp
} // namespace srsgnb
