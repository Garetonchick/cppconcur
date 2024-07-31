#pragma once

#include "fwd.hpp"

#include <wheels/intrusive/list.hpp>

namespace exe::fiber {

// Opaque non-owning handle to the _suspended_ fiber

class FiberHandle : public wheels::IntrusiveListNode<FiberHandle> {
  friend class Fiber;

 public:
  FiberHandle()
      : FiberHandle(nullptr) {
  }

  static FiberHandle Invalid() {
    return FiberHandle(nullptr);
  }

  bool IsValid() const {
    return fiber_ != nullptr;
  }

  // Schedule fiber to the associated scheduler
  void Schedule();

  // Switch to this fiber immediately
  // For symmetric transfer
  void Switch();

 private:
  explicit FiberHandle(Fiber* fiber)
      : fiber_(fiber) {
  }

  Fiber* Release();

 private:
  Fiber* fiber_;
};

}  // namespace exe::fiber
