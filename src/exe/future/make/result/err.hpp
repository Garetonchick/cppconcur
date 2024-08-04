#pragma once

#include <exe/future/type/result.hpp>

#include "../ready.hpp"

#include <exe/result/make/err.hpp>

namespace exe::future {
/*
 * Usage:
 *
 * future::TryFuture<int> auto Attempt() {
 *   return future::Err<int>(TimeoutError());
 * }
 *
 */

template <typename T>
inline auto Err(Error e) {
  return Ready<Result<T>>(result::Err(e));
}

}  // namespace exe::future
