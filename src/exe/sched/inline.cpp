#include <exe/sched/inline.hpp>
#include "exe/sched/task/hint.hpp"

namespace exe::sched {

class Inline : public task::IScheduler {
 public:
  // IScheduler
  void Submit(task::TaskBase* task, task::SchedulerHint) override {
    task->Run();
  }
};

task::IScheduler& Inline() {
  static class Inline instance;
  return instance;
}

}  // namespace exe::sched
