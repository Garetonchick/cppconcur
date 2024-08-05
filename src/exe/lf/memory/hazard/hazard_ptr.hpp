#pragma once

#include <twist/ed/std/atomic.hpp>

namespace exe::lf::memory::hazard {

struct HazardPtr {
  HazardPtr() = default;

  void Set(void* ptr) {
    ptr_.store(ptr);
  }

  void* Get() {
    return ptr_.load();
  }

  void Reset() {
    ptr_.store(nullptr);
  }

 private:
  twist::ed::std::atomic<void*> ptr_{nullptr};
};

}  // namespace exe::lf::memory::hazard
