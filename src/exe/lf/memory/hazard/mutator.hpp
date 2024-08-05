#pragma once

#include "fwd.hpp"
#include "thread.hpp"
#include "guard.hpp"

#include <cstdlib>
#include <thread>

namespace exe::lf::memory::hazard {

class Mutator {
 public:
  Mutator(Manager* manager, ThreadState* thread)
      : manager_(manager),
        thread_(thread) {
  }

  PtrGuard GetHazardPtr(size_t index) {
    return PtrGuard{&(thread_->slots[index])};
  }

  template <typename T>
  void Retire(T* object) {
    thread_->retired.emplace_back(Retired<T>(object));

    if (thread_->retired.size() > scan_freq_) {
      Scan();
    }
  }

  template <typename T>
  void CleanUp() {
    CleanUpImpl();
  }

 private:
  void Scan();
  void CleanUpImpl();

 private:
  const size_t scan_freq_ = 8 * kMaxHazardPtrs * 2;

  [[maybe_unused]] Manager* manager_;
  ThreadState* thread_;
};

}  // namespace exe::lf::memory::hazard
