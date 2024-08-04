#pragma once

#include <exe/result/type/result.hpp>

template <typename T, typename C>
auto operator|(exe::Result<T> r, C c) {
  return c.Pipe(std::move(r));
}
