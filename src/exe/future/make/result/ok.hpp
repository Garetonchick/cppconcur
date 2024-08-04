#pragma once

#include <exe/future/type/result.hpp>

#include "../ready.hpp"

#include <exe/result/make/ok.hpp>

namespace exe::future {

template <typename T>
TryFuture<T> auto Ok(T value) {
  return Ready(result::Ok(std::move(value)));
}

}  // namespace exe::future
