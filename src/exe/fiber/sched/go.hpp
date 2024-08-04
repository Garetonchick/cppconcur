#pragma once

#include <exe/fiber/core/fiber.hpp>
#include <exe/sched/task/function_task.hpp>

namespace exe::fiber {

// Considered harmful

template <typename F>
void Go(IScheduler& scheduler, F func) {
  auto task = sched::task::FunctionTask<F>::New(std::move(func));
  Fiber& fiber = Fiber::NewFiber(scheduler, task);
  fiber.Schedule();
}

template <typename F>
void Go(F func) {
  Go(Fiber::CurrentScheduler(), std::move(func));
}

}  // namespace exe::fiber
