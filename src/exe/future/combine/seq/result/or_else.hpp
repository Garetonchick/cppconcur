#pragma once

#include <exe/future/type/result.hpp>
#include <exe/future/syntax/pipe.hpp>
#include <exe/future/trait/value_of.hpp>
#include <exe/future/thunk/stub.hpp>

#include <exe/result/trait/value_of.hpp>
#include <exe/future/make/result/err.hpp>
#include <exe/future/make/result/ok.hpp>

namespace exe::future {

namespace pipe {

template <typename F>
struct [[nodiscard]] OrElse {
  F user;

  explicit OrElse(F u)
      : user(std::move(u)) {
  }

  // Non-copyable
  OrElse(const OrElse&) = delete;

  template <SomeTryFuture InputFuture>
  SomeTryFuture auto Pipe(InputFuture future) {
    using ResultT = trait::ValueOf<InputFuture>;
    //    using T = result::trait::ValueOf<ResultT>;

    return std::move(future) |
           FlatMap([userfunc = std::move(user)](ResultT val_or_err) {
             if (val_or_err) {
               return future::Ok(std::move(val_or_err.value()));
             }
             return userfunc(std::move(val_or_err.error()));
           });
  }
};

}  // namespace pipe

/*
 * Asynchronous try-catch
 *
 * TryFuture<T> -> (Error -> TryFuture<T>) -> TryFuture<T>
 *
 */

template <typename F>
auto OrElse(F user) {
  return pipe::OrElse{std::move(user)};
}

}  // namespace exe::future
