#include "fiber.hpp"

#include <cstdlib>
#include <twist/ed/static/thread_local/ptr.hpp>

namespace exe::fiber {

TWISTED_STATIC_THREAD_LOCAL_PTR(Fiber, current_fiber);

Fiber& Fiber::NewFiber(IScheduler& scheduler, ITask* task) {
  Fiber* fiber = new Fiber(scheduler, task);
  return *fiber;
}

void Fiber::Suspend(IAwaiter& awaiter) {
  awaiter_ = &awaiter;
  coroutine_.Suspend();
}

void Fiber::Schedule() {
  scheduler_.Submit(this);
}

void Fiber::Run() noexcept {
  current_fiber = this;
  coroutine_.Resume();
  if (coroutine_.IsCompleted()) {
    delete this;
    return;
  }
  awaiter_->Await(FiberHandle(this));
}

void Fiber::Switch() {
  coroutine_.Resume();  // maybe????
}

Fiber& Fiber::Self() {
  return *current_fiber;
}

IScheduler& Fiber::CurrentScheduler() {
  return Self().scheduler_;
}

Fiber::Fiber(IScheduler& scheduler, ITask* task)
    : scheduler_(scheduler),
      coroutine_(task) {
}

}  // namespace exe::fiber
