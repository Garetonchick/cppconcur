#pragma once

#include "state.hpp"

#include <concepts>

namespace exe::future {

// clang-format off

// https://en.wikipedia.org/wiki/Continuation

template <typename C, typename V>
concept Continuation = requires(C cont, V v, State s) {
  { cont.Resume(std::move(v), s) } -> std::same_as<void>;
};

// clang-format on

}  // namespace exe::future
