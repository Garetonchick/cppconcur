#include "handle.hpp"

#include "fiber.hpp"

#include <wheels/core/assert.hpp>

#include <utility>

namespace exe::fiber {

Fiber* FiberHandle::Release() {
  WHEELS_ASSERT(IsValid(), "Invalid fiber handle");
  return std::exchange(fiber_, nullptr);
}

void FiberHandle::Schedule() {
  Release()->Schedule();
}

void FiberHandle::Switch() {
  Release()->Switch();
}

}  // namespace exe::fiber
