#pragma once

#include <exe/sched/task/task.hpp>

#include <twist/ed/std/atomic.hpp>

#include <twist/assist/preempt.hpp>

#include <cassert>
#include <array>
#include <span>

namespace exe::sched::tp::fast {

// SP/MC lock-free ring buffer for local tasks

template <size_t Capacity>
class WorkStealingQueue {
  struct Slot {
    twist::ed::std::atomic<task::TaskBase*> task{nullptr};
  };

 public:
  // Producer

  bool TryPush(task::TaskBase* task) {
    uint64_t current_tail = tail_.load();
    if (current_tail - head_.load() == Capacity) {
      return false;
    }

    buffer_[current_tail % Capacity].task.store(task);
    tail_.fetch_add(1);
    return true;
  }

  // Consumers

  task::TaskBase* TryPop() {
    uint64_t old_head = head_.load();
    task::TaskBase* task = nullptr;
    while (true) {
      if (tail_.load() == old_head) {
        return nullptr;
      }

      task = buffer_[old_head % Capacity].task.load();

      if (head_.compare_exchange_strong(old_head, old_head + 1)) {
        break;
      }
    }

    return task;
  }

  size_t Grab(std::span<task::TaskBase*> out_buffer) {
    while (true) {
      uint64_t new_head = tail_.load();
      uint64_t old_head = head_.load();
      const size_t max_grab = std::min(out_buffer.size(), new_head - old_head);
      new_head = std::min(new_head, old_head + max_grab);

      if (max_grab == 0) {
        return 0;
      }

      for (size_t i = 0; i < max_grab; ++i) {
        out_buffer[i] = buffer_[(old_head + i) % Capacity].task.load();
      }

      while (old_head < new_head) {
        if (head_.compare_exchange_strong(old_head, new_head)) {
          break;
        }
      }

      if (old_head >= new_head) {
        continue;
      }

      size_t grabbed = new_head - old_head;

      for (size_t i = 0; i < grabbed; ++i) {
        out_buffer[i] = out_buffer[max_grab - grabbed + i];
      }

      return grabbed;
    }
  }

  size_t SizeEstimate() {
    size_t old_head = head_.load();
    size_t old_tail = tail_.load();
    return old_tail - old_head;
  }

 private:
  twist::ed::std::atomic_uint64_t head_{0};
  twist::ed::std::atomic_uint64_t tail_{0};
  std::array<Slot, Capacity> buffer_;
};

}  // namespace exe::sched::tp::fast
