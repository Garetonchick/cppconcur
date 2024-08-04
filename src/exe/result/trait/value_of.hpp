#pragma once

#include <exe/result/type/result.hpp>

namespace exe::result::trait {

namespace match {

template <typename T>
struct ValueOf;

template <typename T>
struct ValueOf<Result<T>> {
  using Type = T;
};

}  // namespace match

template <typename Result>
using ValueOf = typename match::ValueOf<Result>::Type;

}  // namespace exe::result::trait
