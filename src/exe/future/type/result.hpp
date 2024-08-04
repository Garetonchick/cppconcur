#pragma once

#include "future.hpp"

#include <exe/result/type/result.hpp>
#include <exe/result/trait/is_result.hpp>

namespace exe::future {

// Fallible Future

template <typename F>
concept SomeTryFuture =
    SomeFuture<F> && result::trait::IsResult<typename F::ValueType>;

template <typename F, typename T>
concept TryFuture = Future<F, Result<T>>;

}  // namespace exe::future
