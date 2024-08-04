#pragma once

#include "stack.hpp"

#include <sure/context.hpp>

#include <exe/sched/task/task.hpp>
#include <twist/assist/fiber.hpp>

namespace exe::fiber {

class Coroutine : private sure::ITrampoline {
  using ITask = sched::task::ITask;

 public:
  explicit Coroutine(ITask*);

  void Resume();
  void Suspend();

  bool IsCompleted() const;

 private:
  void Run() noexcept;

 private:
  static const size_t kStackSize = 1 * 1024 * 1024;

  sure::ExecutionContext coroutine_ctx_;
  sure::ExecutionContext original_ctx_;
  Stack stack_;
  ITask* task_;
  bool completed_{false};

  // Lipovsky's magic fields
  twist::assist::Fiber fiber_;
  twist::assist::FiberHandle caller_fiber_;
};

}  // namespace exe::fiber
