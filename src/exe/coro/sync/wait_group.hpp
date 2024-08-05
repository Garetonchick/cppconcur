#pragma once

#include <coroutine>
#include <mutex>

#include <exe/thread/spinlock.hpp>
#include <exe/sched/task/scheduler.hpp>

#include <wheels/intrusive/forward_list.hpp>

namespace exe::coro {

class WaitGroup {
  struct [[nodiscard]] Waiter : public sched::task::TaskBase {
    friend class WaitGroup;

    explicit Waiter(WaitGroup* wg)
        : wg_(wg) {
    }

    // Awaiter

    void Schedule() {
      if (sched_ == nullptr) {
        Run();
      } else {
        sched_->Submit(this, sched::task::SchedulerHint::UpToYou);
      }
    }

    void Run() noexcept override {
      handle_.resume();
    }

    bool await_ready() {  // NOLINT
      return false;
    }

    template <typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> handle) {  // NOLINT
      handle_ = handle;
      sched_ = handle.promise().GetScheduler();
      return wg_->WaitSuspend(this);
    }

    void await_resume() {  // NOLINT
    }

   private:
    std::coroutine_handle<> handle_{nullptr};
    sched::task::IScheduler* sched_{nullptr};
    WaitGroup* wg_;
  };

 public:
  void Add(size_t count) {
    std::lock_guard guard(lock_);
    n_wait_ += count;
  }

  void Done() {
    std::unique_lock ulock(lock_);
    --n_wait_;

    if (n_wait_ != 0 || waiters_.IsEmpty()) {
      return;
    }

    while (waiters_.Size() > 1) {
      static_cast<Waiter*>(waiters_.PopFront())->Schedule();
    }

    ulock.unlock();
    static_cast<Waiter*>(waiters_.PopFront())->Schedule();
  }

  // Asynchronous
  auto Wait() {
    return Waiter{this};
  }

 private:
  bool WaitSuspend(sched::task::TaskBase* waiter) {
    std::lock_guard guard(lock_);
    if (n_wait_ == 0) {
      return false;
    }
    waiters_.PushBack(waiter);
    return true;
  }

 private:
  thread::SpinLock lock_;
  size_t n_wait_{0};
  wheels::IntrusiveForwardList<sched::task::TaskBase> waiters_;
};

}  // namespace exe::coro
