#pragma once

#include "task.hpp"

namespace exe::sched::task {

template <typename F>
class FunctionTask : task::TaskBase {
 public:
  virtual ~FunctionTask() = default;

  static task::TaskBase* New(F&& f) {
    auto task = new FunctionTask(std::move(f));
    return task;
  }

  // Movable

  FunctionTask(FunctionTask&& o)
      : f_(std::move(o.f_)) {
  }

  FunctionTask& operator=(FunctionTask&& o) {
    f_ = std::move(o.f_);
    return *this;
  }

  // Non-copyable
  FunctionTask(const FunctionTask&&) = delete;
  FunctionTask& operator=(const FunctionTask&&) = delete;

  void Run() noexcept override {
    f_();
    delete this;
  }

 private:
  explicit FunctionTask(F&& f)
      : f_(std::move(f)) {
  }

 private:
  F f_;
};

}  // namespace exe::sched::task
