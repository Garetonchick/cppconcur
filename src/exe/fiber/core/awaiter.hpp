#pragma once

#include <function2/function2.hpp>

#include "handle.hpp"

namespace exe::fiber {

struct IAwaiter {
  virtual ~IAwaiter() = default;
  virtual void Await(FiberHandle) = 0;
};

template <class T>
class Awaiter : public IAwaiter {
 public:
  explicit Awaiter(T&& awaiter)
      : awaiter_(std::move(awaiter)) {
  }
  virtual ~Awaiter() = default;

  // Non-copyable
  Awaiter(const Awaiter&) = delete;
  Awaiter& operator=(const Awaiter&) = delete;

  // Non-movable
  Awaiter(Awaiter&&) = delete;
  Awaiter& operator=(Awaiter&&) = delete;

  void Await(FiberHandle handle) override {
    awaiter_(handle);
  }

 private:
  T awaiter_;
};

}  // namespace exe::fiber
