#pragma once

#include <cstddef>

namespace exe::sched::tp::fast {

class Coordinator {
 public:
  explicit Coordinator(size_t /*threads*/) {
    // Mop implemented
  }

  void NotifyOnSubmit() {
    if (ShouldWakeWorker()) {
      WakeWorker();
    }
  }

  // ???

 private:
  bool ShouldWakeWorker() const;
  void WakeWorker();

 private:
  // ???
};

}  // namespace exe::sched::tp::fast
