#pragma once

#include <exe/future/model/thunk.hpp>

namespace exe::future {

// Represents future value of type T

template <typename F>
concept SomeFuture = Thunk<F>;

template <typename F, typename V>
concept Future = SomeFuture<F> && std::same_as<typename F::ValueType, V>;

}  // namespace exe::future
