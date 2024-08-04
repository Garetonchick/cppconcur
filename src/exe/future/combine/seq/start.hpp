#pragma once

#include <exe/future/syntax/pipe.hpp>
#include <exe/future/thunk/stub.hpp>
#include <exe/future/trait/value_of.hpp>
#include <exe/future/trait/materialize.hpp>

#include <twist/ed/wait/futex.hpp>
#include <twist/ed/std/atomic.hpp>

namespace exe::future {

namespace thunk {

template <SomeFuture Future>
class [[nodiscard]] StartThunk {
 public:
  using ValueType = Future::ValueType;

  class StartComputation;

  class StartContinuation {
   public:
    explicit StartContinuation(StartComputation* comp)
        : comp_(comp) {
    }

    void Resume(ValueType val, State s) {
      comp_->Resume(std::move(val), s);
    }

   private:
    StartComputation* comp_;
  };

  template <Continuation<ValueType> Consumer>
  class StartComputationWithConsumer;

  class StartComputation {
    using Comp = trait::Materialize<Future, StartContinuation>;

    template <Continuation<ValueType> Consumer>
    friend class StartComputationWithConsumer;

   public:
    explicit StartComputation(Future producer)
        : comp_(producer.Materialize(StartContinuation(this))) {
      comp_.Start();
    }

    void Resume(ValueType val, State s) {
      val_.emplace(std::move(val));
      s_ = s;
      auto wake_key = twist::ed::futex::PrepareWake(ready_);
      ready_.store(1);
      twist::ed::futex::WakeOne(wake_key);
    }

   private:
    twist::ed::std::atomic_uint32_t ready_{0};
    std::optional<ValueType> val_;
    State s_;
    Comp comp_;
  };

  template <Continuation<ValueType> Consumer>
  class StartComputationWithConsumer {
    friend StartComputation;

   public:
    StartComputationWithConsumer(Consumer consumer, StartComputation* comp)
        : consumer_(std::move(consumer)),
          comp_(comp) {
    }

    void Start() {
      twist::ed::futex::Wait(comp_->ready_, 0);
      consumer_.Resume(std::move(comp_->val_.value()), comp_->s_);
      delete comp_;
    }

   private:
    Consumer consumer_;
    StartComputation* comp_;
  };

  explicit StartThunk(Future producer) {
    comp_ = new StartComputation(std::move(producer));
  }

  // Non-copyable
  StartThunk(const StartThunk&) = delete;
  StartThunk& operator=(const StartThunk&) = delete;

  StartThunk(StartThunk&&) = default;

  // Thunk
  template <Continuation<ValueType> Consumer>
  Computation auto Materialize(Consumer&& consumer) {
    return StartComputationWithConsumer<Consumer>(
        std::forward<Consumer>(consumer), comp_);
  }

 private:
  StartComputation* comp_;
};

}  // namespace thunk

namespace pipe {

struct [[nodiscard]] Start {
  Start() = default;

  // Non-copyable
  Start(const Start&) = delete;

  template <SomeFuture InputFuture>
  Future<trait::ValueOf<InputFuture>> auto Pipe(InputFuture f) {
    return thunk::StartThunk(std::move(f));
  }
};

}  // namespace pipe

/*
 * Turn lazy future into eager, force thunk evaluation
 *
 */

inline auto Start() {
  return pipe::Start{};
}

}  // namespace exe::future
