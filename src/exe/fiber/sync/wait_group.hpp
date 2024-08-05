#pragma once

#include <exe/fiber/sched/suspend.hpp>
#include <exe/thread/spinlock.hpp>

#include <cstddef>
#include <mutex>

namespace exe::fiber {

class WaitGroup {
 public:
  void Add(size_t count) {
    std::lock_guard guard(lock_);
    running_ += count;
  }

  void Done() {
    std::unique_lock ulock(lock_);
    running_--;
    if (running_ == 0) {
      bool empty = false;
      while (!empty) {
        auto* waiter = waiters_.PopFront();
        if (waiters_.IsEmpty()) {
          empty = true;
          ulock.unlock();
        }
        if (waiter != nullptr) {
          waiter->Schedule(sched::task::SchedulerHint::UpToYou);
        }
      }
    }
  }

  void Wait() {
    FiberHandle handle_copy;
    Awaiter awaiter([this, &handle_copy](auto handle) {
      std::unique_lock ulock(lock_);
      if (running_ == 0) {
        ulock.unlock();
        handle.Schedule(sched::task::SchedulerHint::UpToYou);
        return;
      }

      handle_copy = handle;
      waiters_.PushBack(&handle_copy);
    });

    Suspend(awaiter);
  }

 private:
  thread::SpinLock lock_;
  size_t running_{0};
  wheels::IntrusiveForwardList<FiberHandle> waiters_;
};

}  // namespace exe::fiber
