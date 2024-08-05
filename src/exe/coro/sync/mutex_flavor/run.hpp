#pragma once

#include <coroutine>
#include <mutex>

#include <exe/thread/spinlock.hpp>
#include <exe/sched/task/scheduler.hpp>

#include <wheels/intrusive/forward_list.hpp>

namespace exe::coro {

namespace mutex_flavor {

class RunMutex {
  using Mutex = RunMutex;

  template <typename F>
  struct [[nodiscard]] Waiter : public sched::task::TaskBase {
    friend class RunMutex;

    explicit Waiter(Mutex* mutex, F cs)
        : mutex_(mutex),
          cs_(std::move(cs)) {
    }

    void Run() noexcept override {
      cs_();
      mutex_->Unlock<F>();
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
      if (mutex_->Lock(this)) {
        cs_();
        mutex_->Unlock<F>();
        return false;
      }
      return true;
    }

    void await_resume() {  // NOLINT
    }

   private:
    Mutex* mutex_;
    F cs_;
    std::coroutine_handle<> handle_{nullptr};
    sched::task::IScheduler* sched_{nullptr};
  };

 public:
  template <typename F>
  auto Run(F cs) {
    return Waiter<F>{this, std::move(cs)};
  }

 private:
  bool Lock(sched::task::TaskBase* waiter) {
    std::lock_guard guard(lock_);
    if (!locked_) {
      locked_ = true;
      return true;
    }
    waiters_.PushBack(waiter);
    return false;
  }

  template <typename F>
  void Unlock() {
    std::lock_guard guard(lock_);
    locked_ = false;
    if (waiters_.NonEmpty()) {
      locked_ = true;
      static_cast<Waiter<F>*>(waiters_.PopFront())->Schedule();
    }
  }

 private:
  thread::SpinLock lock_;
  bool locked_{false};
  wheels::IntrusiveForwardList<sched::task::TaskBase> waiters_;
};

}  // namespace mutex_flavor

}  // namespace exe::coro
