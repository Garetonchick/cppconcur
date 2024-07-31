#include "run_loop.hpp"

namespace exe::sched {

void RunLoop::Submit(task::TaskBase* task) {
  task_queue_.PushBack(task);
}

// Run tasks

size_t RunLoop::RunAtMost(size_t limit) {
  size_t runs = 0;
  for (; runs < limit && task_queue_.NonEmpty(); ++runs) {
    task_queue_.PopFront()->Run();
  }
  return runs;
}

size_t RunLoop::Run() {
  size_t runs = 0;
  for (; task_queue_.NonEmpty(); ++runs) {
    task_queue_.PopFront()->Run();
  }
  return runs;
}

}  // namespace exe::sched
