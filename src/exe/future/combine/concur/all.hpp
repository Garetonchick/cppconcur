#pragma once

#include <exe/future/type/future.hpp>

#include <exe/future/trait/value_of.hpp>
#include <exe/future/trait/materialize.hpp>

#include <exe/future/thunk/stub.hpp>

#include <twist/ed/std/atomic.hpp>

#include <tuple>
#include <optional>

namespace exe::future {

namespace thunk {

template <SomeFuture Future1, SomeFuture Future2>
class [[nodiscard]] BothThunk {
 public:
  BothThunk(Future1 prod1, Future2 prod2)
      : prod1_(std::move(prod1)),
        prod2_(std::move(prod2)) {
  }

  // Non-copyable
  BothThunk(const BothThunk&) = delete;
  BothThunk& operator=(const BothThunk&) = delete;

  BothThunk(BothThunk&&) = default;

  using A = Future1::ValueType;
  using B = Future2::ValueType;
  using ValueType = std::tuple<A, B>;

  template <Continuation<ValueType> Consumer>
  class BothContinuation {
   public:
    explicit BothContinuation(Consumer consumer)
        : consumer_(std::move(consumer)) {
    }

    void ResumeA(A val, State) {
      a_.emplace(std::move(val));
      Check();
    }

    void ResumeB(B val, State) {
      b_.emplace(std::move(val));
      Check();
    }

    void Check() {
      if (finished_.fetch_add(1) == 1) {
        consumer_.Resume(
            std::make_tuple(std::move(a_.value()), std::move(b_.value())),
            State{});
      }
    }

   private:
    std::optional<A> a_;
    std::optional<B> b_;
    Consumer consumer_;
    twist::ed::std::atomic_uint32_t finished_{0};
  };

  template <Continuation<ValueType> Consumer>
  struct ContinuationBinder {
    ContinuationBinder(BothContinuation<Consumer>* cont, bool first)
        : cont(cont),
          first(first) {
    }

    void Resume(auto val, State s) {
      if (first) {
        cont->ResumeA(std::move(val), s);
      } else {
        cont->ResumeB(std::move(val), s);
      }
    }

    BothContinuation<Consumer>* cont;
    bool first;
  };

  template <Continuation<ValueType> Consumer>
  class BothComputation {
    using Comp1 = trait::Materialize<Future1, ContinuationBinder<Consumer>>;
    using Comp2 = trait::Materialize<Future2, ContinuationBinder<Consumer>>;

   public:
    BothComputation(Consumer consumer, Future1 prod1, Future2 prod2)
        : consumer_(std::move(consumer)),
          comp1_(prod1.Materialize(
              ContinuationBinder<Consumer>(&consumer_, true))),
          comp2_(prod2.Materialize(
              ContinuationBinder<Consumer>(&consumer_, false))) {
    }

    void Start() {
      comp1_.Start();
      comp2_.Start();
    }

   private:
    BothContinuation<Consumer> consumer_;  // order is important
    Comp1 comp1_;
    Comp2 comp2_;
  };

  // Thunk
  template <Continuation<ValueType> Consumer>
  Computation auto Materialize(Consumer&& consumer) {
    return BothComputation<Consumer>(std::forward<Consumer>(consumer),
                                     std::move(prod1_), std::move(prod2_));
  }

 private:
  Future1 prod1_;
  Future2 prod2_;
};

}  // namespace thunk

template <SomeFuture LeftInput, SomeFuture RightInput>
SomeFuture auto Both(LeftInput f1, RightInput f2) {
  //  using A = trait::ValueOf<LeftInput>;
  //  using B = trait::ValueOf<RightInput>;

  return thunk::BothThunk(std::move(f1), std::move(f2));
}

}  // namespace exe::future
