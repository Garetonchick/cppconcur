#pragma once

#include <exe/future/contract-state/rendezvous.hpp>
#include <exe/util/manual_lifetime.hpp>

#include <exe/sched/task/submit.hpp>

#include <exe/sched/inline.hpp>

#include <function2/function2.hpp>

namespace exe::future::state {

// Future state for future::Contract

template <typename T>
using Callback = fu2::unique_function<void(T)>;

template <typename T>
class ContractState {
 public:
  static ContractState<T>* New() {
    ContractState<T>* state = nullptr;
    try {
      state = new ContractState<T>();
    } catch (...) {
      delete state;
      throw;
    }
    return state;
  }

  void Produce(T value) {
    value_.Init(std::move(value));
    if (state_.Produce()) {
      callback_(std::move(value_.Access()));
      delete this;
    }
  }

  void Consume(Callback<T> callback) {
    callback_ = std::move(callback);
    if (state_.Consume()) {
      callback_(std::move(value_.Access()));
      delete this;
    }
  }

 private:
  ContractState() = default;

 private:
  ManualLifetime<T> value_;
  Callback<T> callback_;
  RendezvousStateMachine state_;
};

}  // namespace exe::future::state
