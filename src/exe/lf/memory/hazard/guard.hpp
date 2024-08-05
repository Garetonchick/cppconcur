#pragma once

#include "hazard_ptr.hpp"

#include <twist/ed/std/atomic.hpp>

#include <twist/trace/scope.hpp>

namespace exe::lf::memory::hazard {

struct PtrGuard {
  friend class Mutator;

 public:
  PtrGuard(const PtrGuard&) = delete;  // do we need copying????
  PtrGuard& operator=(const PtrGuard&) = delete;

  // Non-movable
  PtrGuard(PtrGuard&&) = delete;
  PtrGuard& operator=(PtrGuard&&) = delete;

  template <typename T>
  T* Protect(twist::ed::std::atomic<T*>& atomic_ptr,
             twist::trace::Scope = twist::trace::Scope(guard, "Protect")) {
    T* obj = atomic_ptr.load();
    while (true) {
      slot_->Set(obj);
      auto upd_obj = atomic_ptr.load();
      if (obj == upd_obj) {
        break;
      }
      obj = upd_obj;
    }
    return obj;
  }

  template <typename T>
  void Announce(T* ptr,
                twist::trace::Scope = twist::trace::Scope(guard, "Announce")) {
    slot_->Set(ptr);
  }

  void Reset(twist::trace::Scope = twist::trace::Scope(guard, "Reset")) {
    slot_->Reset();
  }

  ~PtrGuard() {
    Reset();
  }

 private:
  explicit PtrGuard(HazardPtr* slot)
      : slot_(slot) {
  }

 private:
  // Tracing
  static inline twist::trace::Domain guard{"HazardPtr"};

 private:
  HazardPtr* slot_;
};

}  // namespace exe::lf::memory::hazard
