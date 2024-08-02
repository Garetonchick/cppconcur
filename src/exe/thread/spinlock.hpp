#pragma once

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/wait/spin.hpp>

#include <cstdint>

namespace exe::thread {

class SpinLock {
  using Ticket = uint64_t;

 public:
  // Do not change this method
  void Lock() {
    const Ticket this_thread_ticket = next_free_ticket_.fetch_add(1);

    twist::ed::SpinWait spin_wait;
    while (this_thread_ticket != owner_ticket_.load()) {
      spin_wait();
    }
  }

  bool TryLock() {
    const Ticket next_thread_ticket = owner_ticket_.fetch_sub(1);
    if (next_free_ticket_.load() == next_thread_ticket) {
      return true;
    }
    owner_ticket_.fetch_add(1);
    return false;
  }

  // Do not change this method
  void Unlock() {
    // Do we actually need atomic increment here?
    owner_ticket_.fetch_add(1);
  }

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::ed::std::atomic<Ticket> next_free_ticket_{0};
  twist::ed::std::atomic<Ticket> owner_ticket_{0};
};

}  // namespace exe::thread
