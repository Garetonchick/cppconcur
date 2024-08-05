#pragma once

#include <exe/sched/task/task.hpp>

#include <exe/thread/spinlock.hpp>

#include <wheels/intrusive/forward_list.hpp>

#include <span>
#include <mutex>

namespace exe::sched::tp::fast {

// Unbounded task queue shared between workers

class GlobalQueue {
  using List = wheels::IntrusiveForwardList<task::TaskBase>;

  // using Lock = twist::ed::std::mutex;
  using Lock = exe::thread::SpinLock;

 public:
  void PushOne(task::TaskBase* task) {
    std::lock_guard guard(lock_);
    queue_.PushBack(task);
  }

  void PushMany(std::span<task::TaskBase*> buffer) {
    List buf_list;

    for (auto* task : buffer) {
      buf_list.PushBack(task);
    }

    {
      std::lock_guard guard(lock_);
      queue_.Append(buf_list);  // O(1)
    }
  }

  task::TaskBase* TryPopOne() {
    std::lock_guard guard(lock_);
    return queue_.PopFront();
  }

  // TODO: optimize?
  size_t Grab(std::span<task::TaskBase*> out_buffer) {
    std::lock_guard guard(lock_);
    size_t n_grab = std::min(out_buffer.size(), queue_.Size());

    for (size_t i = 0; i < n_grab; ++i) {
      out_buffer[i] = queue_.PopFront();
    }

    return n_grab;
  }

 private:
  List queue_;
  Lock lock_;
};

}  // namespace exe::sched::tp::fast
