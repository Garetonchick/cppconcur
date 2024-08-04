#pragma once

#include <wheels/intrusive/forward_list.hpp>

namespace exe::sched::task {

struct ITask {
  virtual void Run() noexcept = 0;

 protected:
  ~ITask() = default;
};

// Intrusive task
struct TaskBase : ITask,
                  wheels::IntrusiveForwardListNode<TaskBase> {
  //
};

}  // namespace exe::sched::task
