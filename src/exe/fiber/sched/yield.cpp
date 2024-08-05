#include "yield.hpp"

#include "suspend.hpp"

namespace exe::fiber {

void Yield() {
  Awaiter awaiter([](auto handle) {
    handle.Schedule(sched::task::SchedulerHint::Yield);
  });
  Suspend(awaiter);
}

}  // namespace exe::fiber
