#pragma once

#include <exe/future/model/thunk.hpp>

#include <function2/function2.hpp>

namespace exe::future {

namespace thunk {

template <typename V>
struct Box {
  //  struct ProducerStorageBase {
  //    virtual ~ProducerStorageBase() = default;
  //    virtual void MaterializeComputation(void* computation, void* consumer) =
  //    0;
  //  };
  //
  //  template <Thunk Producer>
  //  struct ProducerStorage : public ProducerStorageBase {
  //
  //    template<Continuation<V> Consumer>
  //    auto Materialize(Consumer consumer) {
  //      return producer.Materialize(std::move(consumer));
  //    }
  //
  //    void MaterializeComputation(void* computation, void* consumer) override
  //    {
  //
  //    }
  //
  //    Producer producer;
  //  };

  using ValueType = V;

  class BoxConsumer {
   public:
    template <Continuation<V> Consumer>
    explicit BoxConsumer(Consumer cons) {
      resume_ = [consumer = std::move(cons)](V val, State s) mutable {
        consumer.Resume(std::move(val), s);
      };
    }

    void Resume(V val, State s) {
      resume_(std::move(val), s);
    }

   private:
    fu2::unique_function<void(V, State)> resume_;
  };

  class BoxComputation {
   public:
    template <Computation Comp>
    explicit BoxComputation(Comp comp) {
      start_ = [computation = std::move(comp)]() mutable {
        computation.Start();
      };
    }

    void Start() {
      start_();
    }

   private:
    fu2::unique_function<void()> start_;
  };

  template <Thunk Producer>
  explicit Box(Producer&& producer) {
    materialize_ = [prod = std::move(producer)](BoxConsumer cons) mutable {
      return BoxComputation(prod.Materialize(std::move(cons)));
    };
  }

  template <Continuation<V> Consumer>
  Computation auto Materialize(Consumer&& cons) {
    return materialize_(BoxConsumer(std::move(cons)));
  }

 private:
  fu2::unique_function<BoxComputation(BoxConsumer)> materialize_;
};

}  // namespace thunk

}  // namespace exe::future
