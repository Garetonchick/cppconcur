#pragma once

#include <exe/future/type/future.hpp>

#include <exe/future/thunk/box.hpp>

namespace exe::future {

// Boxed Future

template <typename V>
using BoxedFuture = thunk::Box<V>;

}  // namespace exe::future
