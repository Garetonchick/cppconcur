#pragma once

#include <exe/thread/spinlock.hpp>
#include <exe/fiber/sched/suspend.hpp>

namespace exe::fiber {

// One-shot

class Event {
 public:
  void Wait() {
    FiberHandle handle_copy;
    Awaiter awaiter([this, &handle_copy](auto handle) {
      std::unique_lock ulock(lock_);
      if (fired_) {
        ulock.unlock();  // Unlock BEFORE Schedule
        handle.Schedule();
        return;
      }
      handle_copy = handle;
      waiters_.PushBack(&handle_copy);
    });
    Suspend(awaiter);
  }

  void Fire() {
    std::unique_lock ulock(lock_);
    fired_ = true;
    while (true) {
      auto waiter = waiters_.PopFront();
      bool empty = false;
      if (waiters_.IsEmpty()) {
        ulock.unlock();
        empty = true;
      }
      if (waiter != nullptr) {
        // After this Suspend exits in Wait and then handle_copy is destroyed
        // Moreover, if this is the last waiter, whole Event can be destroyed
        // So, you must not access any Event fields after last Schedule
        waiter->Schedule();
      }
      if (empty) {
        break;
      }
    }
  }

 private:
  wheels::IntrusiveForwardList<FiberHandle> waiters_;
  thread::SpinLock lock_;
  bool fired_{false};
};

}  // namespace exe::fiber
