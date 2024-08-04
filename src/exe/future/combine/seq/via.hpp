#pragma once

#include <exe/sched/task/scheduler.hpp>

#include <exe/future/syntax/pipe.hpp>
#include <exe/future/trait/value_of.hpp>
#include <exe/future/thunk/stub.hpp>
#include <exe/future/run/func_demand.hpp>

namespace exe::future {

namespace thunk {

template <SomeFuture Future>
class [[nodiscard]] ViaThunk {
 public:
  explicit ViaThunk(Future producer, sched::task::IScheduler& scheduler)
      : producer_(std::move(producer)),
        scheduler_(scheduler) {
  }

  // Non-copyable
  ViaThunk(const ViaThunk&) = delete;
  ViaThunk& operator=(const ViaThunk&) = delete;

  ViaThunk(ViaThunk&&) = default;

  using ValueType = Future::ValueType;

  //  template <Continuation<ValueType> Consumer, Computation Comp>
  //  class ViaComputation {
  //   public:
  //    ViaComputation(Future producer, Consumer consumer,
  //    sched::task::IScheduler& scheduler)
  //        : producer_(std::move(producer)),
  //          consumer_(std::move(consumer)),
  //          scheduler_(scheduler) {
  //
  //      computation_ = producer_.Materialize(FuncDemand([this](auto val, State
  //      s){
  //        s.scheduler = this->scheduler_;
  //        consumer_.Resume(std::move(val), s);
  //      }));
  //    }
  //
  //    void Start() {
  //      computation_.Start();
  //    }
  //
  //   private:
  //    Comp computation_;
  //    Future producer_;
  //    Consumer consumer_;
  //    sched::task::IScheduler& scheduler_;
  //  };

  // Thunk
  template <Continuation<ValueType> Consumer>
  Computation auto Materialize(Consumer&& consumer) {
    return producer_.Materialize(
        FuncDemand([consumer = std::forward<Consumer>(consumer),
                    scheduler = &scheduler_](auto val, State s) mutable {
          s.scheduler = scheduler;
          consumer.Resume(std::move(val), s);
        }));
  }

 private:
  Future producer_;
  sched::task::IScheduler& scheduler_;
};

}  // namespace thunk

namespace pipe {

struct [[nodiscard]] Via {
  sched::task::IScheduler* scheduler;

  explicit Via(sched::task::IScheduler& s)
      : scheduler(&s) {
  }

  // Non-copyable
  Via(const Via&) = delete;

  template <SomeFuture InputFuture>
  Future<trait::ValueOf<InputFuture>> auto Pipe(InputFuture future) {
    return thunk::ViaThunk<InputFuture>(std::move(future), *scheduler);
  }
};

}  // namespace pipe

/*
 * Scheduling state
 *
 * Future<T> -> Scheduler -> Future<T>
 *
 */

inline auto Via(sched::task::IScheduler& scheduler) {
  return pipe::Via{scheduler};
}

}  // namespace exe::future
