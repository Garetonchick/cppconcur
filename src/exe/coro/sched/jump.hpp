#pragma once

#include <exe/sched/task/scheduler.hpp>

#include <coroutine>
#include "exe/sched/task/hint.hpp"

namespace exe::coro {

namespace detail {

struct [[nodiscard]] JumpToAwaiter : public sched::task::TaskBase {
  explicit JumpToAwaiter(sched::task::IScheduler& sched)
      : sched_(sched) {
  }

  // Awaiter

  bool await_ready() {  // NOLINT
    return false;
  }

  template <typename Promise>
  void await_suspend(std::coroutine_handle<Promise> handle) {  // NOLINT
    promise_ = handle.address();
    handle.promise().SetScheduler(&sched_);
    sched_.Submit(this, sched::task::SchedulerHint::UpToYou);
  }

  void Run() noexcept override {
    std::coroutine_handle<>::from_address(promise_).resume();
  }

  void await_resume() {  // NOLINT
  }

 private:
  sched::task::IScheduler& sched_;
  void* promise_{nullptr};
};

}  // namespace detail

inline auto JumpTo(sched::task::IScheduler& target) {
  return detail::JumpToAwaiter{target};
}

}  // namespace exe::coro
