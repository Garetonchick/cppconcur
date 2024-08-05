#pragma once

#include <exe/sched/task/scheduler.hpp>

#include <coroutine>

namespace exe::coro {

namespace detail {

struct CurrentSchedulerAwaiter {
  bool await_ready() {  // NOLINT
    return false;
  }

  template <typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> handle) {  // NOLINT
    sched_ = handle.promise().GetScheduler();
    return false;
  }

  sched::task::IScheduler* await_resume() {  // NOLINT
    return sched_;
  }

 private:
  sched::task::IScheduler* sched_{nullptr};
};

}  // namespace detail

inline auto CurrentScheduler() {
  return detail::CurrentSchedulerAwaiter{};
}

}  // namespace exe::coro
