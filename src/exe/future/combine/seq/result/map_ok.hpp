#pragma once

#include <exe/future/type/result.hpp>
#include <exe/future/syntax/pipe.hpp>
#include <exe/future/trait/value_of.hpp>
#include <exe/future/thunk/stub.hpp>

#include <exe/result/trait/value_of.hpp>
#include <exe/future/make/result/err.hpp>
#include <exe/future/make/result/ok.hpp>

#include <type_traits>

namespace exe::future {

namespace thunk {

template <SomeFuture Future, typename F>
class [[nodiscard]] MapOkThunk {
 public:
  MapOkThunk(Future producer, F func)
      : producer_(std::move(producer)),
        func_(std::move(func)) {
  }

  // Non-copyable
  MapOkThunk(const MapOkThunk&) = delete;
  MapOkThunk& operator=(const MapOkThunk&) = delete;

  MapOkThunk(MapOkThunk&&) = default;

  using ResultT = trait::ValueOf<Future>;
  using T = result::trait::ValueOf<ResultT>;
  using U = std::invoke_result_t<F, T>;
  using ValueType = Result<U>;

  template <Continuation<ValueType> Consumer>
  class MapOkContinuation : public sched::task::TaskBase {
   public:
    MapOkContinuation(Consumer consumer, F func)
        : consumer_(std::move(consumer)),
          func_(std::move(func)) {
    }

    void Resume(ResultT val, State s) {
      s_ = s;
      val_ = std::move(val);
      if (s.scheduler == nullptr) {
        Run();
      } else {
        s.scheduler->Submit(this);
      }
    }

    void Run() noexcept override {
      if (val_.has_value()) {
        auto res = func_(std::move(val_.value()));
        consumer_.Resume(result::Ok(std::move(res)), s_);
      } else {
        consumer_.Resume(result::Err(std::move(val_.error())), s_);
      }
    }

   private:
    ResultT val_;
    State s_;
    Consumer consumer_;
    F func_;
  };

  // Thunk
  template <Continuation<ValueType> Consumer>
  Computation auto Materialize(Consumer&& consumer) {
    return producer_.Materialize(MapOkContinuation<Consumer>(
        std::forward<Consumer>(consumer), std::move(func_)));
  }

 private:
  Future producer_;
  F func_;
};

}  // namespace thunk

namespace pipe {

template <typename F>
struct [[nodiscard]] MapOk {
  F user;

  explicit MapOk(F&& u)
      : user(std::move(u)) {
  }

  // Non-copyable
  MapOk(const MapOk&) = delete;

  template <SomeTryFuture InputFuture>
  SomeTryFuture auto Pipe(InputFuture future) {
    //    using ResultT = trait::ValueOf<InputFuture>;
    //    using T = result::trait::ValueOf<ResultT>;
    //    using U = std::invoke_result_t<F, T>;

    return thunk::MapOkThunk(std::move(future), std::move(user));
  }
};

}  // namespace pipe

// TryFuture<T> -> (T -> U) -> TryFuture<U>

template <typename F>
auto MapOk(F user) {
  return pipe::MapOk(std::move(user));
}

}  // namespace exe::future
