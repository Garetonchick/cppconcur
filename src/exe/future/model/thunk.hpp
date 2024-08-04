#pragma once

#include "cont.hpp"
#include "comp.hpp"

namespace exe::future {

// Concrete continuation
template <typename V>
struct Demand {
  void Resume(V, State) {
  }
};

// Thunk
// https://wiki.haskell.org/Thunk

template <typename T>
concept Thunk = requires(T thunk, Demand<typename T::ValueType> demand) {
  typename T::ValueType;

  { thunk.Materialize(std::move(demand)) } -> Computation;
};

}  // namespace exe::future
