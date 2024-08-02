#pragma once

#include <exe/fiber/sched/suspend.hpp>
#include <exe/thread/spinlock.hpp>
#include <vector>
#include <mutex>

#include <fmt/core.h>

namespace exe::fiber {

// One-shot

// class Event {
//  public:
//   void Wait() {
//     FiberHandle handle_copy;
//
//     lock_.lock();
//     if (fired_) {
//       lock_.unlock();
//       return;
//     }
//
//     Awaiter awaiter([this, &handle_copy](auto handle) {
//       handle_copy = handle;
//       waiters_head_.PushBack(&handle_copy);
//       lock_.unlock();
//     });
//     Suspend(awaiter);
//   }
//
//   void Fire() {
//     std::unique_lock ulock(lock_);
//     fired_ = true;
//     while(true) {
//       auto waiter = waiters_head_.PopFront();
//       bool empty = false;
//       if(waiters_head_.IsEmpty()) {
//           ulock.unlock();
//           empty = true;
//       }
//       if(waiter != nullptr) {
//         // After this Suspend exits in Wait and then handle_copy is destroyed
//         // Moreover, if this is the last waiter, whole Event can be destroyed
//         // So, you must not access any Event fields after last Schedule
//         waiter->Schedule();
//       }
//       if(empty) {
//         break;
//       }
//     }
//   }
//
//   ~Event() {
//     std::lock_guard guard(lock_);
//   }
//
//  private:
//   wheels::IntrusiveList<FiberHandle> waiters_head_;
//   thread::SpinLock lock_;
//   bool fired_{false};
// };

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
  wheels::IntrusiveList<FiberHandle> waiters_;
  thread::SpinLock lock_;
  bool fired_{false};
};


}  // namespace exe::fiber
