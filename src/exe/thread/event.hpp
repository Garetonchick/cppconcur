#pragma once

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/wait/futex.hpp>

namespace exe::thread {

class Event {
 public:
  // One-shot
  void Fire() {
    auto wake_key = twist::ed::futex::PrepareWake(fired_);
    fired_.store(1);
    twist::ed::futex::WakeOne(wake_key);
  }

  void Wait() {
    if (fired_.load() == 0) {
      twist::ed::futex::Wait(fired_, 0);
    }
  }

 private:
  twist::ed::std::atomic_uint32_t fired_{0};
};

}  // namespace exe::thread
