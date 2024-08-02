#pragma once

#include <cstddef>
#include <optional>

namespace exe::fiber {

struct Hint {
  size_t stack_size;
};

}  // namespace exe::fiber
