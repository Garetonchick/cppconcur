#pragma once

#include <exe/future/combine/concur/all.hpp>

#include <exe/future/type/future.hpp>

/*
 * Syntactic sugar for future::Both combinator
 *
 * Usage:
 *
 * auto both = std::move(lhs) * std::move(rhs);
 *
 */

template <exe::future::SomeFuture L, exe::future::SomeFuture R>
exe::future::SomeFuture auto operator*(L lhs, R rhs) {
  return exe::future::Both(std::move(lhs), std::move(rhs));
}
