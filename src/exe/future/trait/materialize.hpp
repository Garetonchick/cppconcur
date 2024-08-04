#pragma once

#include <exe/future/model/thunk.hpp>

#include "value_of.hpp"

#include <utility>

namespace exe::future {

namespace trait {

template <SomeFuture Future, Continuation<trait::ValueOf<Future>> Demand>
using Materialize =
    decltype(std::declval<Future>().Materialize(std::declval<Demand>()));

}  // namespace trait

}  // namespace exe::future
