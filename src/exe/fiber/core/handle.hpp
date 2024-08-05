#pragma once

#include "exe/sched/task/hint.hpp"
#include "fwd.hpp"

#include <wheels/intrusive/forward_list.hpp>

namespace exe::fiber {

// Opaque non-owning handle to the _suspended_ fiber

class FiberHandle : public wheels::IntrusiveForwardListNode<FiberHandle> {
  friend class Fiber;

 public:
  FiberHandle()
      : FiberHandle(nullptr) {
  }

  explicit FiberHandle(Fiber* fiber)
      : fiber_(fiber) {
  }

  static FiberHandle Invalid() {
    return FiberHandle(nullptr);
  }

  bool IsValid() const {
    return fiber_ != nullptr;
  }

  // Schedule fiber to the associated scheduler
  void Schedule(sched::task::SchedulerHint);

  // Switch to this fiber immediately
  // For symmetric transfer
  void Switch();

 private:
  Fiber* Release();

 private:
  Fiber* fiber_;
};

}  // namespace exe::fiber
