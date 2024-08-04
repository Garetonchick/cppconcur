#pragma once

#include <exe/future/type/result.hpp>
#include <exe/future/syntax/pipe.hpp>
#include <exe/future/trait/value_of.hpp>
#include <exe/future/thunk/stub.hpp>

#include <exe/result/trait/value_of.hpp>
#include <exe/future/combine/seq/result/map_ok.hpp>
#include <exe/future/make/result/err.hpp>
#include <exe/future/make/result/ok.hpp>

namespace exe::future {

// namespace thunk {
//
// template <SomeFuture Future, typename F>
// class [[nodiscard]] AndThenThunk {
//  public:
//   AndThenThunk(Future producer, F func)
//       : producer_(std::move(producer)),
//         func_(std::move(func)) {
//   }
//
//   // Non-copyable
//   AndThenThunk(const AndThenThunk&) = delete;
//   AndThenThunk& operator=(const AndThenThunk&) = delete;
//
//   AndThenThunk(AndThenThunk&&) = default;
//
//   using ResultT = trait::ValueOf<Future>;
//   using T = result::trait::ValueOf<ResultT>;
//   using U = std::invoke_result_t<F, T>;
//   using ValueType = Result<U>;
//
//   template <Continuation<ValueType> Consumer>
//   class AndThenContinuation : public sched::task::TaskBase {
//    public:
//     AndThenContinuation(Consumer consumer, F func)
//         : consumer_(std::move(consumer)),
//           func_(std::move(func)) {
//     }
//
//     void Resume(ResultT val, State s) {
//       s_ = s;
//       val_ = std::move(val);
//       if (s.scheduler == nullptr) {
//         Run();
//       } else {
//         s.scheduler->Submit(this);
//       }
//     }
//
//     void Run() noexcept override {
//       if(val_.has_value()) {
//         auto res = func_(std::move(val_.value()));
//         consumer_.Resume(result::Ok(std::move(res)), s_);
//       } else {
//         consumer_.Resume(result::Err(std::move(val_.error())), s_);
//       }
//     }
//
//    private:
//     ResultT val_;
//     State s_;
//     Consumer consumer_;
//     F func_;
//   };
//
//   // Thunk
//   template <Continuation<ValueType> Consumer>
//   Computation auto Materialize(Consumer&& consumer) {
//     return producer_.Materialize(AndThenContinuation<Consumer>(
//         std::forward<Consumer>(consumer), std::move(func_)));
//   }
//
//  private:
//   Future producer_;
//   F func_;
// };
//
// }

namespace pipe {

template <typename F>
struct [[nodiscard]] AndThen {
  F user;

  explicit AndThen(F u)
      : user(std::move(u)) {
  }

  // Non-copyable
  AndThen(const AndThen&) = delete;

  template <SomeTryFuture InputFuture>
  SomeTryFuture auto Pipe(InputFuture future) {
    using ResultT = trait::ValueOf<InputFuture>;
    using T = result::trait::ValueOf<ResultT>;
    //    using FutureU = std::invoke_result_t<F, T>;
    //    using ResultU = trait::ValueOf<FutureU>;
    //    using U = result::trait::ValueOf<ResultU>;

    return std::move(future) |
           FlatMap([userfunc = std::move(user)](ResultT val) {
             if (val) {
               return userfunc(std::move(val.value()));
             }
             return future::Err<T>(std::move(val.error()));
           });
  }
};

}  // namespace pipe

/*
 * Asynchronous try-catch
 *
 * TryFuture<T> -> (T -> TryFuture<U>) -> TryFuture<U>
 *
 */

template <typename F>
auto AndThen(F user) {
  return pipe::AndThen{std::move(user)};
}

}  // namespace exe::future
