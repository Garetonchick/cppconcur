#pragma once

#include <exe/future/type/future.hpp>

/*
 * Pipeline operator for sequential Future composition
 *
 * Usage:
 *
 * auto f = future::Value(7)
 *          | future::Map([](int v) { return v + 1; });
 *
 */

template <exe::future::SomeFuture F, typename C>
auto operator|(F f, C c) {
  return c.Pipe(std::move(f));
}
