#pragma once

#include "body.hpp"
#include "coroutine.hpp"
#include "scheduler.hpp"
#include "awaiter.hpp"

namespace exe::fiber {

// Fiber = stackful coroutine + scheduler

class Fiber : private sched::task::TaskBase {
 public:
  virtual ~Fiber() = default; 
  static Fiber& NewFiber(IScheduler&, ITask*);

  void Suspend(IAwaiter&);

  void Schedule(sched::task::SchedulerHint);
  void Switch();

  static Fiber& Self();
  static IScheduler& CurrentScheduler();

 private:
  Fiber(IScheduler&, ITask*);

  void Run() noexcept override;

 private:
  IScheduler& scheduler_;
  Coroutine coroutine_;
  IAwaiter* awaiter_{nullptr};
};

}  // namespace exe::fiber
