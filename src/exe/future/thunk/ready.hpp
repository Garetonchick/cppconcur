#pragma once

#include <exe/future/model/thunk.hpp>

namespace exe::future {

namespace thunk {

template <typename V>
struct [[nodiscard]] Ready {
  V value;

  explicit Ready(V v)
      : value(std::move(v)) {
  }

  // Non-copyable
  Ready(const Ready&) = delete;
  Ready& operator=(const Ready&) = delete;

  Ready(Ready&&) = default;

  // Computation
  template <Continuation<V> Consumer>
  struct ComputeValueFor {
    Consumer consumer;
    V value;

    void Start() {
      consumer.Resume(std::move(value), State{});
    }
  };

  // Thunk

  using ValueType = V;

  template <Continuation<V> Consumer>
  Computation auto Materialize(Consumer&& c) {
    return ComputeValueFor<Consumer>{std::forward<Consumer>(c),
                                     std::move(value)};
  }
};

}  // namespace thunk

}  // namespace exe::future
