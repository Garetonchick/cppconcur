#pragma once

#include <exe/future/type/future.hpp>

#include <exe/future/thunk/stub.hpp>
#include <exe/future/contract-state/contract.hpp>

#include <cassert>
#include <tuple>

namespace exe::future {

namespace thunk {

template <typename V>
class [[nodiscard]] ContractThunk {
 public:
  explicit ContractThunk(state::ContractState<V>* cont_state)
      : cont_state_(cont_state) {
  }

  // Non-copyable
  ContractThunk(const ContractThunk&) = delete;
  ContractThunk& operator=(const ContractThunk&) = delete;

  ContractThunk(ContractThunk&&) = default;

  using ValueType = V;

  template <Continuation<ValueType> Consumer>
  class ContractComputation {
   public:
    explicit ContractComputation(Consumer consumer,
                                 state::ContractState<V>* cont_state)
        : consumer_(std::move(consumer)),
          cont_state_(cont_state) {
    }

    void Start() {
      cont_state_->Consume([this](auto val) {
        consumer_.Resume(std::move(val), State{});
      });
    }

   private:
    Consumer consumer_;
    state::ContractState<V>* cont_state_;
  };

  // Thunk
  template <Continuation<ValueType> Consumer>
  Computation auto Materialize(Consumer&& consumer) {
    return ContractComputation<Consumer>(std::move(consumer), cont_state_);
  }

 private:
  state::ContractState<V>* cont_state_{nullptr};
};

}  // namespace thunk

// Producer

template <typename T>
class Promise {
 public:
  explicit Promise(state::ContractState<T>* cont_state)
      : cont_state_(cont_state) {
  }

  // Non-copyable
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  // Move-constructible
  Promise(Promise&& o)
      : cont_state_(o.cont_state_) {
    o.cont_state_ = nullptr;
  }

  // Non-move-assignable
  Promise& operator=(Promise&&) = delete;

  ~Promise() {
  }

  // One-shot
  void Set(T val) && {
    cont_state_->Produce(std::move(val));
  }

 private:
  state::ContractState<T>* cont_state_;
};

/*
 * Asynchronous one-shot contract
 *
 * Usage:
 *
 * auto [f, p] = future::Contract<int>();
 *
 * // Producer
 * std::move(p).Set(7);
 *
 * // Consumer
 * auto v = future::Get(std::move(f));  // 7
 *
 */

template <typename T>
std::tuple<thunk::ContractThunk<T>, Promise<T>> Contract() {
  auto* cont_state = state::ContractState<T>::New();
  return {thunk::ContractThunk(cont_state), Promise(cont_state)};
}

}  // namespace exe::future
