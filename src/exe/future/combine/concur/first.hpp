#pragma once

#include <exe/future/type/future.hpp>

#include <exe/future/trait/value_of.hpp>
#include <exe/future/trait/materialize.hpp>

#include <exe/future/thunk/stub.hpp>
#include <twist/ed/wait/futex.hpp>
#include <twist/ed/std/atomic.hpp>

#include <function2/function2.hpp>

#include <tuple>
#include <optional>
#include <iostream>

namespace exe::future {

namespace thunk {

template <SomeFuture Future1, SomeFuture Future2>
class [[nodiscard]] FirstThunk {
 public:
  FirstThunk(Future1 prod1, Future2 prod2)
      : prod1_(std::move(prod1)),
        prod2_(std::move(prod2)) {
  }

  // Non-copyable
  FirstThunk(const FirstThunk&) = delete;
  FirstThunk& operator=(const FirstThunk&) = delete;

  FirstThunk(FirstThunk&&) = default;

  using ValueType = Future1::ValueType;

  template <Continuation<ValueType> Consumer>
  class FirstContinuation {
   public:
    FirstContinuation(Consumer consumer,
                      fu2::function_view<void(void*)> destroy_comp,
                      void* destroy_ptr)
        : consumer_(std::move(consumer)),
          destroy_comp_(destroy_comp),
          destroy_ptr_(destroy_ptr) {
    }

    void Resume1(ValueType val, State s) {
      Resume(std::move(val), s);
    }

    void Resume2(ValueType val, State s) {
      Resume(std::move(val), s);
    }

    void Resume(ValueType val, State s) {
      if (finished_.fetch_add(1) == 0) {
        s.scheduler = nullptr;
        consumer_.Resume(std::move(val), s);
      } else {
        // delete
        destroy_comp_(destroy_ptr_);
      }
    }

   private:
    Consumer consumer_;
    twist::ed::std::atomic_uint32_t finished_{0};
    fu2::function_view<void(void*)> destroy_comp_;
    void* destroy_ptr_;
  };

  template <Continuation<ValueType> Consumer>
  struct ContinuationBinder {
    explicit ContinuationBinder(FirstContinuation<Consumer>* cont, bool first)
        : cont(cont),
          first(first) {
    }

    void Resume(ValueType val, State s) {
      if (first) {
        cont->Resume1(std::move(val), s);
      } else {
        cont->Resume2(std::move(val), s);
      }
    }

    FirstContinuation<Consumer>* cont;
    bool first;
  };

  template <Continuation<ValueType> Consumer>
  class FirstComputation {
    using Comp1 = trait::Materialize<Future1, ContinuationBinder<Consumer>>;
    using Comp2 = trait::Materialize<Future2, ContinuationBinder<Consumer>>;

   public:
    FirstComputation(Consumer consumer, Future1 prod1, Future2 prod2)
        : consumer_(
              std::move(consumer),
              [](void* ptr) {
                delete (FirstComputation*)ptr;
              },
              this),
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
    FirstContinuation<Consumer> consumer_;  // order is important
    Comp1 comp1_;
    Comp2 comp2_;
  };

  template <typename Comp>
  struct ComputationWrapper {
    explicit ComputationWrapper(Comp& comp)
        : comp(comp) {
    }

    void Start() {
      comp.Start();
    }

    Comp& comp;
  };

  // Thunk
  template <Continuation<ValueType> Consumer>
  Computation auto Materialize(Consumer&& consumer) {
    auto comp = new FirstComputation<Consumer>(
        std::forward<Consumer>(consumer), std::move(prod1_), std::move(prod2_));
    return ComputationWrapper<FirstComputation<Consumer>>(*comp);
  }

 private:
  Future1 prod1_;
  Future2 prod2_;
};

}  // namespace thunk

template <SomeFuture LeftInput, SomeFuture RightInput>
SomeFuture auto First(LeftInput prod1, RightInput prod2) {
  //  using T = trait::ValueOf<LeftInput>;

  return thunk::FirstThunk(std::move(prod1), std::move(prod2));
}

}  // namespace exe::future
