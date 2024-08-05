#pragma once

#include "scheduler.hpp"
#include "function_task.hpp"

namespace exe::sched::task {

template <typename F>
void Submit(IScheduler& scheduler, F fun) {
  auto task = FunctionTask<F>::New(std::move(fun));
  scheduler.Submit(task, SchedulerHint::UpToYou);
}

}  // namespace exe::sched::task
