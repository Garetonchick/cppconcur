#pragma once

#include "state.hpp"

#include <concepts>

namespace exe::future {

// clang-format off

template <typename C>
concept Computation = requires(C comp) {
  { comp.Start() } -> std::same_as<void>;
};

// clang-format on

}  // namespace exe::future
