#pragma once

#include <exe/future/type/future.hpp>

#include <exe/future/syntax/pipe.hpp>

#include <exe/future/trait/value_of.hpp>
#include <exe/future/trait/materialize.hpp>

#include "func_demand.hpp"

#include <cstdlib>  // std::abort

namespace exe::future {

template <typename T, typename V>
struct DestructingDemand {
  explicit DestructingDemand(T* addr)
      : addr(addr) {
  }

  void Resume(V, State) {
    delete addr;
  }

  T* addr;
};

template <SomeFuture Future>
class SelfDestructingComputation {
  using V = Future::ValueType;
  using Comp = decltype(std::declval<Future&>().Materialize(
      std::declval<DestructingDemand<SelfDestructingComputation, V>&&>()));

 public:
  explicit SelfDestructingComputation(Future future)
      : computation_(future.Materialize(
            DestructingDemand<SelfDestructingComputation, V>(this))) {
    computation_.Start();
  }

 private:
  Comp computation_;
};

template <SomeFuture Future>
void Detach(Future future) {
  new SelfDestructingComputation(std::move(future));
}

// Chaining

namespace pipe {

struct [[nodiscard]] Detach {
  template <SomeFuture Future>
  void Pipe(Future f) {
    future::Detach(std::move(f));
  }
};

}  // namespace pipe

inline auto Detach() {
  return pipe::Detach{};
}

}  // namespace exe::future
