#pragma once

#include <exe/fiber/core/fiber.hpp>
#include <exe/sched/task/function_task.hpp>
#include <exe/fiber/core/hint.hpp>
#include <exe/fiber/core/scheduler.hpp>

namespace exe::fiber {

template <typename F>
void Go(IScheduler& scheduler, F func, Hint hint = {}) {
  (void)hint;  // nolint
  auto task = sched::task::FunctionTask<F>::New(std::move(func));
  Fiber& fiber = Fiber::NewFiber(scheduler, task);
  fiber.Schedule();
}

template <typename F>
void Go(F func, Hint hint = {}) {
  (void)hint;  // nolint
  Go(Fiber::CurrentScheduler(), std::move(func));
}

}  // namespace exe::fiber
