#pragma once

namespace exe::sched::task {

enum class SchedulerHint {
  UpToYou = 1,  // Rely on scheduler decision
  Next = 2,     // Use LIFO scheduling
  Yield = 3,    // Yield control to another task chain
};

}  // namespace exe::sched::task
