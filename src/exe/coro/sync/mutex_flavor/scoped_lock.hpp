#pragma once

#include <coroutine>
#include <mutex>

#include <exe/thread/spinlock.hpp>

#include <wheels/intrusive/forward_list.hpp>
#include <exe/sched/task/scheduler.hpp>

namespace exe::coro {

namespace mutex_flavor {

class ScopedLockMutex {
  using Mutex = ScopedLockMutex;

  struct Waiter;

 public:
  class LockGuard {
    friend struct Waiter;

   public:
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

    ~LockGuard() {
      mutex_->Unlock();
    }

   private:
    explicit LockGuard(Mutex* mutex)
        : mutex_(mutex) {
    }

   private:
    Mutex* mutex_;
  };

 private:
  struct [[nodiscard]] Waiter : sched::task::TaskBase {
    explicit Waiter(Mutex* m)
        : mutex_(m) {
    }

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
      return !mutex_->Lock(this);
    }

    LockGuard await_resume() {  // NOLINT
      return LockGuard{mutex_};
    }

   private:
    Mutex* mutex_;
    std::coroutine_handle<> handle_{nullptr};
    sched::task::IScheduler* sched_{nullptr};
  };

 public:
  // Asynchronous
  auto ScopedLock() {
    return Waiter{this};
  }

 private:
  // Synchronous
  void Unlock() {
    std::lock_guard guard(lock_);
    locked_ = false;
    if (waiters_.NonEmpty()) {
      locked_ = true;
      static_cast<Waiter*>(waiters_.PopFront())->Schedule();
    }
  }

  bool Lock(sched::task::TaskBase* waiter) {
    std::lock_guard guard(lock_);
    if (!locked_) {
      locked_ = true;
      return true;
    }
    waiters_.PushBack(waiter);
    return false;
  }

 private:
  thread::SpinLock lock_;
  bool locked_{false};
  wheels::IntrusiveForwardList<sched::task::TaskBase> waiters_;
};

}  // namespace mutex_flavor

}  // namespace exe::coro
