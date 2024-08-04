#pragma once

#include <exe/future/type/future.hpp>

#include <exe/future/thunk/ready.hpp>

namespace exe::future {

/*
 * Ready value
 *
 * Usage:
 *
 * auto f = future::Ready(result::Ok(42));
 *
 */

template <typename V>
Future<V> auto Ready(V value) {
  return thunk::Ready<V>{std::move(value)};
}

}  // namespace exe::future
