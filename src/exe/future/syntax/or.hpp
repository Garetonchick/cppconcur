#pragma once

#include <exe/future/combine/concur/first.hpp>

#include <exe/future/type/future.hpp>

/*
 * Syntactic sugar for future::First combinator
 *
 * Usage:
 *
 * auto first = std::move(lhs) or std::move(rhs);
 *
 */

template <exe::future::SomeFuture L, exe::future::SomeFuture R>
exe::future::SomeFuture auto operator||(L lhs, R rhs) {
  return exe::future::First(std::move(lhs), std::move(rhs));
}
