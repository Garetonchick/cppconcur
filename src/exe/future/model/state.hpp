#pragma once

#include <exe/sched/task/scheduler.hpp>

#include <exe/sched/inline.hpp>

namespace exe::future {

// Mutable state

struct State {
  sched::task::IScheduler* scheduler = nullptr;
};

}  // namespace exe::future
