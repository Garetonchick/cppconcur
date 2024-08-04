#pragma once

#include <exe/future/type/future.hpp>

namespace exe::future {

namespace trait {

template <SomeFuture Future>
using ValueOf = typename Future::ValueType;

}  // namespace trait

}  // namespace exe::future
