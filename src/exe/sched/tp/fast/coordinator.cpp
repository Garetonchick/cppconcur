#include "coordinator.hpp"

namespace exe::sched::tp::fast {

bool Coordinator::ShouldWakeWorker() const {
  return false;  // Mop implemented
}

void Coordinator::WakeWorker() {
  // Mop implemented
}

}  // namespace exe::sched::tp::fast
