#pragma once

#include <exe/sched/task/task.hpp>
#include <exe/sched/task/hint.hpp>

namespace exe::sched::task {

struct IScheduler {
  virtual ~IScheduler() = default;

  virtual void Submit(TaskBase* task, SchedulerHint) = 0;
};

}  // namespace exe::sched::task
