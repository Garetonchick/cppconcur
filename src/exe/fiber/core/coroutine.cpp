#include "coroutine.hpp"

namespace exe::fiber {

Coroutine::Coroutine(ITask* task)
    : stack_(Stack::AllocateBytes(kStackSize)),
      task_(task) {
  coroutine_ctx_.Setup(stack_.MutView(), this);

  // magic
  twist::assist::NewFiber(&fiber_);
}

void Coroutine::Resume() {
  // magic
  caller_fiber_ = twist::assist::SwitchToFiber(fiber_.Handle());

  original_ctx_.SwitchTo(coroutine_ctx_);
}

void Coroutine::Suspend() {
  // magic
  twist::assist::SwitchToFiber(caller_fiber_);

  coroutine_ctx_.SwitchTo(original_ctx_);
}

void Coroutine::Run() noexcept {
  task_->Run();
  completed_ = true;
  twist::assist::SwitchToFiber(caller_fiber_);
  coroutine_ctx_.ExitTo(original_ctx_);
  WHEELS_UNREACHABLE();
}

bool Coroutine::IsCompleted() const {
  return completed_;
}


}  // namespace exe::fiber
