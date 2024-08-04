#pragma once

#include <exe/result/type/result.hpp>
#include <exe/result/type/status.hpp>

namespace exe::result {

template <typename T>
Result<T> Ok(T value) {
  return {std::move(value)};
}

Status Ok() {
  return {Unit{}};
}

}  // namespace exe::result
