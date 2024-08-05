#pragma once

#include <coroutine>
#include <mutex>

#include <exe/sched/task/scheduler.hpp>
#include <exe/thread/spinlock.hpp>
#include <twist/ed/std/atomic.hpp>
#include "exe/sched/task/hint.hpp"

namespace exe::coro {

class Event {
  struct [[nodiscard]] Waiter : public sched::task::TaskBase {
    friend class Event;

    explicit Waiter(Event* event)
        : event_(event) {
    }

    // Awaiter
    void Run() noexcept override {
      handle_.resume();
    }

    void Schedule() {
      if (sched_ == nullptr) {
        Run();
      } else {
        sched_->Submit(this, sched::task::SchedulerHint::UpToYou);
      }
    }

    bool await_ready() {  // NOLINT
      return false;
    }

    template <typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> handle) {  // NOLINT
      handle_ = handle;
      sched_ = handle.promise().GetScheduler();
      {
        std::lock_guard guard(event_->lock_);
        if (event_->fired_) {
          return false;
        }

        event_->waiters_.PushBack(this);
      }
      return true;
    }

    void await_resume() {  // NOLINT
    }

   private:
    Event* event_{nullptr};
    std::coroutine_handle<> handle_{nullptr};
    sched::task::IScheduler* sched_{nullptr};
  };

 public:
  // Asynchronous
  auto Wait() {
    return Waiter{this};
  }

  // One-shot
  void Fire() {
    {
      std::lock_guard guard(lock_);
      fired_ = true;

      if (waiters_.IsEmpty()) {
        return;
      }
    }
    while (waiters_.Size() > 1) {
      static_cast<Waiter*>(waiters_.PopFront())->Schedule();
    }
    static_cast<Waiter*>(waiters_.PopFront())->Schedule();
  }

 private:
  thread::SpinLock lock_;
  bool fired_{false};
  wheels::IntrusiveForwardList<sched::task::TaskBase> waiters_;
};

}  // namespace exe::coro
