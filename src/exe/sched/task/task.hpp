#pragma once

#include <wheels/intrusive/forward_list.hpp>

namespace exe::sched::task {

struct ITask {
  virtual ~ITask() = default;

  virtual void Run() noexcept = 0;
};

// Intrusive task
struct TaskBase : ITask,
                  wheels::IntrusiveForwardListNode<TaskBase> {
  //
};

}  // namespace exe::sched::task
