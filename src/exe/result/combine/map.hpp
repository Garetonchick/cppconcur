#pragma once

#include <exe/result/type/result.hpp>
#include <exe/result/trait/value_of.hpp>
#include <exe/result/syntax/pipe.hpp>

namespace exe::result {

namespace pipe {

template <typename F>
struct [[nodiscard]] Map {
  F user;

  explicit Map(F u)
      : user(std::move(u)) {
  }

  // Non-copyable
  Map(const Map&) = delete;
  Map& operator=(const Map&) = delete;

  template <typename T>
  using U = std::invoke_result_t<F, T>;

  template <typename T>
  Result<U<T>> Pipe(Result<T> r) {
    return std::move(r).transform(user);
  }
};

}  // namespace pipe

/*
 * Result<T> -> (T -> U) -> Result<U>
 *
 * Usage:
 *
 * auto r = result::Ok(1) | result::Map([](int v) {
 *   return v + 1;
 * });
 *
 */

template <typename F>
auto Map(F user) {
  return pipe::Map{std::move(user)};
}

}  // namespace exe::result
