#pragma once

#include <exe/future/type/result.hpp>

#include <exe/result/trait/is_result.hpp>
#include <exe/result/make/ok.hpp>

#include <exe/future/combine/seq/map.hpp>

#include <type_traits>

namespace exe::future {

// Explicit

/*
 * Future<T> -> TryFuture<T'> where
 *    T' = U for T = Result<U>
 *    T' = T otherwise
 */

template <SomeTryFuture Future>
auto AsTryFuture(Future f) {
  return std::move(f);  // Identity
}

template <SomeFuture Future,
          std::enable_if_t<!result::trait::IsResult<trait::ValueOf<Future>>,
                           bool> = true>
SomeTryFuture auto AsTryFuture(Future f) {
  using V = trait::ValueOf<Future>;

  return std::move(f) | future::Map([](V v) {
           return result::Ok(v);
         });
}

// Chaining

namespace pipe {

struct [[nodiscard]] AsTryFuture {
  AsTryFuture() = default;

  // Non-copyable
  AsTryFuture(const AsTryFuture&) = delete;

  template <SomeFuture Future>
  SomeTryFuture auto Pipe(Future f) {
    return future::AsTryFuture(std::move(f));
  }
};

}  // namespace pipe

auto AsTryFuture() {
  return pipe::AsTryFuture{};
}

}  // namespace exe::future
