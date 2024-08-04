#pragma once

#include <exe/sched/task/scheduler.hpp>

namespace exe::sched {

// Executes task immediately on the current thread

task::IScheduler& Inline();

}  // namespace exe::sched
