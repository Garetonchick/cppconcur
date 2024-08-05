#pragma once

#include <exe/future/syntax/pipe.hpp>
#include <exe/future/trait/value_of.hpp>
#include <exe/future/thunk/stub.hpp>

#include <exe/future/run/func_demand.hpp>

#include <optional>
#include "exe/sched/task/hint.hpp"

namespace exe::future {

namespace thunk {

template <SomeFuture Future, typename F>
class [[nodiscard]] MapThunk {
 public:
  MapThunk(Future producer, F func)
      : producer_(std::move(producer)),
        func_(std::move(func)) {
  }

  // Non-copyable
  MapThunk(const MapThunk&) = delete;
  MapThunk& operator=(const MapThunk&) = delete;

  MapThunk(MapThunk&&) = default;

  using ValueType = std::invoke_result_t<F, typename Future::ValueType>;

  template <Continuation<ValueType> Consumer>
  class MapContinuation : public sched::task::TaskBase {
   public:
    MapContinuation(Consumer consumer, F func)
        : consumer_(std::move(consumer)),
          func_(std::move(func)) {
    }

    void Resume(auto val, State s) {
      if (s.scheduler == nullptr) {
        auto res = func_(std::move(val));
        consumer_.Resume(std::move(res), s);
        return;
      }
      s_ = s;
      val_.emplace(std::move(val));
      s.scheduler->Submit(this, sched::task::SchedulerHint::UpToYou);
    }

    void Run() noexcept override {
      auto res = func_(std::move(val_.value()));
      consumer_.Resume(std::move(res), s_);
    }

   private:
    std::optional<typename Future::ValueType> val_;
    State s_;
    Consumer consumer_;
    F func_;
  };

  // Thunk
  template <Continuation<ValueType> Consumer>
  Computation auto Materialize(Consumer&& consumer) {
    return producer_.Materialize(MapContinuation<Consumer>(
        std::forward<Consumer>(consumer), std::move(func_)));
  }

 private:
  Future producer_;
  F func_;
};

}  // namespace thunk

namespace pipe {

template <typename F>
struct [[nodiscard]] Map {
  F user;

  explicit Map(F u)
      : user(std::move(u)) {
  }

  // Non-copyable
  Map(const Map&) = delete;

  template <SomeFuture InputFuture>
  SomeFuture auto Pipe(InputFuture future) {
    //    using T = trait::ValueOf<InputFuture>;
    //    using U = std::invoke_result_t<F, T>;

    return thunk::MapThunk(std::move(future), std::move(user));
  }
};

}  // namespace pipe

/*
 * Functor
 * https://wiki.haskell.org/Typeclassopedia
 *
 * Future<T> -> (T -> U) -> Future<U>
 *
 */

template <typename F>
auto Map(F user) {
  return pipe::Map{std::move(user)};
}

}  // namespace exe::future
