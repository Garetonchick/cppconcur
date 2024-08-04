#pragma once

#include <exe/result/type/result.hpp>
#include <exe/result/syntax/pipe.hpp>

namespace exe::result {

namespace pipe {

template <typename F>
struct [[nodiscard]] OrElse {
  F user;

  explicit OrElse(F u)
      : user(std::move(u)) {
  }

  // Non-copyable
  OrElse(const OrElse&) = delete;
  OrElse& operator=(const OrElse&) = delete;

  template <typename T>
  Result<T> Pipe(Result<T> r) {
    return std::move(r).or_else(user);
  }
};

}  // namespace pipe

/*
 * Result<T> -> (Error -> Result<T>) -> Result<T>
 *
 * Usage:
 *
 * auto failure = [] -> Result<int> {
 *   return result::Err(TimeoutError());
 * }
 *
 * auto r = failure() | result::OrElse([](Error) {
 *   return result::Ok(42);  // Fallback
 * });
 *
 */

template <typename F>
auto OrElse(F user) {
  return pipe::OrElse{std::move(user)};
}

}  // namespace exe::result
