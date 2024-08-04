#include <exe/sched/inline.hpp>

namespace exe::sched {

class Inline : public task::IScheduler {
 public:
  // IScheduler
  void Submit(task::TaskBase* task) override {
    task->Run();
  }
};

task::IScheduler& Inline() {
  static class Inline instance;
  return instance;
}

}  // namespace exe::sched
