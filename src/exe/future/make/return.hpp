#pragma once

#include "ready.hpp"

namespace exe::future {

/*
 * return for Future monad
 * https://wiki.haskell.org/Typeclassopedia
 *
 * Usage:
 *
 * auto f = future::Return(11);
 *
 */

template <typename V>
Future<V> auto Return(V value) {
  return Ready(std::move(value));
}

}  // namespace exe::future
