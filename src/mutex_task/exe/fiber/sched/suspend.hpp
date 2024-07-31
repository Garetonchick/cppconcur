#pragma once

#include <exe/fiber/core/awaiter.hpp>

namespace exe::fiber {

void Suspend(IAwaiter&);

}  // namespace exe::fiber
