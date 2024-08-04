#pragma once

#include <exe/future/type/future.hpp>

#include <exe/future/syntax/pipe.hpp>

#include <exe/future/trait/value_of.hpp>
#include <exe/future/trait/materialize.hpp>
#include <exe/thread/event.hpp>

#include "func_demand.hpp"

#include <cstdlib>  // std::abort

namespace exe::future {

/*
 * Unwrap Future synchronously (blocking current thread)
 *
 * Usage:
 *
 * future::Get(future::Submit(pool, [] { return 7; }));
 *
 */

template <SomeFuture Future>
trait::ValueOf<Future> Get(Future future) {
  std::optional<trait::ValueOf<Future>> res;
  thread::Event ready;

  auto computation = future.Materialize(FuncDemand([&](auto val, State) {
    res.emplace(std::move(val));
    ready.Fire();
  }));

  computation.Start();
  ready.Wait();
  return std::move(res.value());
}

// Chaining

namespace pipe {

struct [[nodiscard]] Get {
  template <SomeFuture Future>
  trait::ValueOf<Future> Pipe(Future f) {
    return future::Get(std::move(f));
  }
};

}  // namespace pipe

/*
 * Usage:
 *
 * auto v = future::Submit(pool, { return 7; }) | future::Get();
 */

inline auto Get() {
  return pipe::Get{};
}

}  // namespace exe::future
