#pragma once

#include <exe/future/model/state.hpp>

namespace exe::future {

template <typename F>
class FuncDemand {
 public:
  explicit FuncDemand(F func)
      : func_(std::move(func)) {
  }

  template <typename V>
  void Resume(V val, State state) {
    func_(std::move(val), state);
  }

 private:
  F func_;
};

}  // namespace exe::future