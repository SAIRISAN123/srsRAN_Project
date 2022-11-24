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

#include "../task_schedulers/ue_task_scheduler.h"
#include "srsgnb/cu_cp/cu_cp.h"
#include "srsgnb/cu_cp/du_processor.h"

namespace srsgnb {
namespace srs_cu_cp {

class du_processor_to_cu_cp_task_scheduler : public du_processor_ue_task_scheduler
{
public:
  du_processor_to_cu_cp_task_scheduler() {}

  void connect_cu_cp(ue_task_scheduler& cu_cp_task_sched_) { cu_cp_task_sched = &cu_cp_task_sched_; }

  void schedule_async_task(du_index_t du_index, ue_index_t ue_index, async_task<void>&& task) override
  {
    srsgnb_assert(cu_cp_task_sched != nullptr, "CU-CP task scheduler handler must not be nullptr");
    cu_cp_task_sched->handle_ue_async_task(du_index, ue_index, std::move(task));
  }

  unique_timer make_unique_timer() override
  {
    srsgnb_assert(cu_cp_task_sched != nullptr, "CU-CP task scheduler handler must not be nullptr");
    return cu_cp_task_sched->make_unique_timer();
  }
  timer_manager& get_timer_manager() override
  {
    srsgnb_assert(cu_cp_task_sched != nullptr, "CU-CP task scheduler handler must not be nullptr");
    return cu_cp_task_sched->get_timer_manager();
  }

private:
  ue_task_scheduler* cu_cp_task_sched = nullptr;
};

class du_processor_cu_cp_adapter : public du_processor_cu_cp_notifier
{
public:
  void connect_cu_cp(cu_cp_du_handler& cu_cp_mng_) { cu_cp_handler = &cu_cp_mng_; }

  void on_rrc_ue_created(du_index_t du_index, ue_index_t ue_index, rrc_ue_interface* rrc_ue) override
  {
    srsgnb_assert(cu_cp_handler != nullptr, "CU-CP handler must not be nullptr");
    cu_cp_handler->handle_rrc_ue_creation(du_index, ue_index, rrc_ue);
  }

private:
  cu_cp_du_handler* cu_cp_handler = nullptr;
};

} // namespace srs_cu_cp
} // namespace srsgnb
