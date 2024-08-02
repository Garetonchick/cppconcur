#include "suspend.hpp"

#include <exe/fiber/core/fiber.hpp>

namespace exe::fiber {

void Suspend(IAwaiter& awaiter) {
  Fiber::Self().Suspend(awaiter);
}

}  // namespace exe::fiber
