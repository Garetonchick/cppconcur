#pragma once

#include <exe/future/syntax/pipe.hpp>
#include <exe/future/trait/value_of.hpp>
#include <exe/future/thunk/stub.hpp>

#include <optional>

namespace exe::future {

namespace thunk {

template <SomeFuture FFuture, typename V>
class [[nodiscard]] FlattenThunk {
 public:
  explicit FlattenThunk(FFuture producer)
      : producer_(std::move(producer)) {
  }

  // Non-copyable
  FlattenThunk(const FlattenThunk&) = delete;
  FlattenThunk& operator=(const FlattenThunk&) = delete;

  FlattenThunk(FlattenThunk&&) = default;

  using ValueType = V;

  template <Continuation<ValueType> Consumer>
  class FlattenContinuation {
    class InnerFlattenContinuation {
     public:
      InnerFlattenContinuation(Consumer consumer, State s)
          : consumer_(std::move(consumer)),
            s_(s) {
      }

      void Resume(auto val, State) {
        consumer_.Resume(std::move(val), s_);
      }

     private:
      Consumer consumer_;
      State s_;
    };

    //    using Comp = decltype(std::declval<typename
    //    FFuture::ValueType&>().Materialize(std::declval<Consumer&&>()));
    using Comp =
        decltype(std::declval<typename FFuture::ValueType&>().Materialize(
            std::declval<InnerFlattenContinuation&&>()));

   public:
    explicit FlattenContinuation(Consumer consumer)
        : consumer_(std::move(consumer)) {
    }

    void Resume(SomeFuture auto future_val, State s) {
      //      computation_.emplace(future_val.Materialize(std::move(consumer_)));
      auto inner_cont = InnerFlattenContinuation(std::move(consumer_), s);
      computation_.emplace(future_val.Materialize(std::move(inner_cont)));
      computation_.value().Start();
    }

   private:
    std::optional<Comp> computation_;
    Consumer consumer_;
  };

  // Thunk
  template <Continuation<ValueType> Consumer>
  Computation auto Materialize(Consumer&& consumer) {
    return producer_.Materialize(
        FlattenContinuation<Consumer>(std::forward<Consumer>(consumer)));
  }

 private:
  FFuture producer_;
};

}  // namespace thunk

namespace pipe {

struct [[nodiscard]] Flatten {
  Flatten() = default;

  // Non-copyable
  Flatten(const Flatten&) = delete;

  template <SomeFuture InputFuture>
  SomeFuture auto Pipe(InputFuture future) {
    using FutureT = trait::ValueOf<InputFuture>;
    using T = trait::ValueOf<FutureT>;

    return thunk::FlattenThunk<InputFuture, T>(std::move(future));
  }
};

}  // namespace pipe

/*
 * Collapse nested Future-s
 *
 * Future<Future<T>> -> Future<T>
 *
 */

inline auto Flatten() {
  return pipe::Flatten{};
}

}  // namespace exe::future
