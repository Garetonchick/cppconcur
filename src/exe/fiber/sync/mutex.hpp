#pragma once

#include <exe/fiber/sched/suspend.hpp>
#include <exe/thread/spinlock.hpp>

#include <mutex>
#include "exe/sched/task/hint.hpp"

namespace exe::fiber {

class Mutex {
 public:
  void Lock() {
    FiberHandle handle_copy;
    Awaiter awaiter([this, &handle_copy](auto handle) {
      std::lock_guard guard(lock_);

      if (!locked_) {
        locked_ = true;
        handle.Schedule(sched::task::SchedulerHint::UpToYou);
        return;
      }

      handle_copy = handle;
      waiters_.PushBack(&handle_copy);
    });
    Suspend(awaiter);
  }

  bool TryLock() {
    std::lock_guard guard(lock_);

    if (!locked_) {
      locked_ = true;
      return true;
    }
    return false;
  }

  void Unlock() {
    std::lock_guard guard(lock_);
    locked_ = false;
    if (waiters_.NonEmpty()) {
      locked_ = true;
      waiters_.PopFront()->Schedule(sched::task::SchedulerHint::UpToYou);
    }
  }

  // Lockable

  void lock() {  // NOLINT
    Lock();
  }

  bool try_lock() {  // NOLINT
    return TryLock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  thread::SpinLock lock_;
  bool locked_{false};
  wheels::IntrusiveForwardList<FiberHandle> waiters_;
};

}  // namespace exe::fiber
