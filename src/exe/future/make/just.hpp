#pragma once

#include <exe/future/type/future.hpp>

#include <exe/future/thunk/stub.hpp>

#include <exe/unit.hpp>

namespace exe::future {

/*
 * Ready unit value
 * https://en.wikipedia.org/wiki/Unit_type
 *
 * Usage:
 *
 * auto f = future::Just()
 *          | future::Via(pool)
 *          | future::Map([](Unit) { return 4; });
 *
 */

namespace thunk {

class [[nodiscard]] JustThunk {
 public:
  JustThunk() = default;
  // Non-copyable
  JustThunk(const JustThunk&) = delete;
  JustThunk& operator=(const JustThunk&) = delete;

  JustThunk(JustThunk&&) = default;

  using ValueType = Unit;

  template <Continuation<ValueType> Consumer>
  class JustComputation {
   public:
    explicit JustComputation(Consumer consumer)
        : consumer_(std::move(consumer)){};

    void Start() {
      consumer_.Resume(Unit{}, State{});
    }

   private:
    Consumer consumer_;
  };

  // Thunk
  template <Continuation<ValueType> Consumer>
  Computation auto Materialize(Consumer&& consumer) {
    return JustComputation<Consumer>(std::forward<Consumer>(consumer));
  }
};

}  // namespace thunk

inline Future<Unit> auto Just() {
  return thunk::JustThunk{};
}

}  // namespace exe::future
