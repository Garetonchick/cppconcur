#pragma once

#include <coroutine>

#include <exe/sched/task/scheduler.hpp>
#include "exe/sched/task/hint.hpp"

namespace exe::coro {

namespace detail {

struct YieldAwaiter : public sched::task::TaskBase {
  YieldAwaiter() = default;

  // Awaiter

  bool await_ready() {  // NOLINT
    return false;
  }

  template <typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> handle) {  // NOLINT
    handle_ = handle;
    sched::task::IScheduler* sched = handle.promise().GetScheduler();
    if (sched == nullptr) {
      return false;
    }
    sched->Submit(this, sched::task::SchedulerHint::Yield);
    return true;
  }

  void Run() noexcept override {
    handle_.resume();
  }

  void await_resume() {  // NOLINT
  }

 private:
  std::coroutine_handle<> handle_{nullptr};
};

}  // namespace detail

// Precondition: coroutine is running in `current` scheduler
inline auto Yield() {
  return detail::YieldAwaiter{};
}

}  // namespace exe::coro
