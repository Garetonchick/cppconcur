#pragma once

#include <exe/future/type/future.hpp>

#include <exe/future/thunk/stub.hpp>

#include <type_traits>
#include "exe/sched/task/hint.hpp"

namespace exe::future {

/*
 * Computation (to be) scheduled to the given task scheduler
 *
 * Usage:
 *
 * auto f = future::Submit(pool, [] {
 *   return 42;  // ~ computation
 * });
 *
 */

namespace thunk {

template <typename F>
class [[nodiscard]] SubmitThunk {
 public:
  explicit SubmitThunk(sched::task::IScheduler& scheduler, F func)
      : scheduler_(scheduler),
        func_(std::move(func)) {
  }

  // Non-copyable
  SubmitThunk(const SubmitThunk&) = delete;
  SubmitThunk& operator=(const SubmitThunk&) = delete;

  SubmitThunk(SubmitThunk&&) = default;

  using ValueType = std::invoke_result_t<F>;

  template <Continuation<ValueType> Consumer>
  class SubmitComputation : public sched::task::TaskBase {
   public:
    SubmitComputation(Consumer consumer, sched::task::IScheduler& scheduler,
                      F func)
        : consumer_(std::move(consumer)),
          scheduler_(scheduler),
          func_(std::move(func)) {
    }

    void Start() {
      scheduler_.Submit(this, sched::task::SchedulerHint::UpToYou);
    }

   private:
    void Run() noexcept override {
      consumer_.Resume(func_(), State{&scheduler_});
    }

   private:
    Consumer consumer_;
    sched::task::IScheduler& scheduler_;
    F func_;
  };

  // Thunk
  template <Continuation<ValueType> Consumer>
  Computation auto Materialize(Consumer&& consumer) {
    return SubmitComputation<Consumer>(std::forward<Consumer>(consumer),
                                       scheduler_, std::move(func_));
  }

 private:
  sched::task::IScheduler& scheduler_;
  F func_;
};

}  // namespace thunk

template <typename F>
Future<std::invoke_result_t<F>> auto Submit(sched::task::IScheduler& scheduler,
                                            F user) {
  return thunk::SubmitThunk<F>(scheduler, std::move(user));
}

}  // namespace exe::future
