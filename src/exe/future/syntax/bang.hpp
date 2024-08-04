#pragma once

#include <exe/future/combine/seq/start.hpp>

/*
 * "Bang" operator (!)
 *
 * Named after bang patterns in Strict Haskell
 * https://www.fpcomplete.com/haskell/tutorial/all-about-strictness/
 *
 * Turns lazy Future into eager one / starts operation
 *
 */

template <exe::future::SomeFuture Future>
auto operator!(Future f) {
  return std::move(f) | exe::future::Start();
}
