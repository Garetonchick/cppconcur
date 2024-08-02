#pragma once

#include <cstddef>
#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>

namespace exe::thread {

class WaitGroup {
 public:
  void Add(size_t count) {
    std::lock_guard guard(mutex_);
    counter_ += count;
  }

  void Done() {
    std::lock_guard guard(mutex_);
    --counter_;
    if (counter_ == 0 && contention_ != 0) {
      CV.notify_all();
    }
  }

  void Wait() {
    std::unique_lock lock(mutex_);
    ++contention_;
    CV.wait(lock, [this]() {
      return counter_ == 0;
    });
    --contention_;
  }

 private:
  twist::ed::std::condition_variable CV;  // NOLINT
  twist::ed::std::mutex mutex_;
  size_t counter_{0};
  size_t contention_{0};
};

}  // namespace exe::thread
