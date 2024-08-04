#pragma once

#include <twist/ed/std/thread.hpp>

#include <exe/sched/task/scheduler.hpp>
#include <exe/thread/queue.hpp>

#include <cstddef>
#include <vector>

namespace exe::sched {

class ThreadPool : public task::IScheduler {
 public:
  explicit ThreadPool(size_t threads);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Non-movable
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  void Start();

  // task::IScheduler
  void Submit(task::TaskBase*) override;

  static ThreadPool* Current();

  void Stop();

 private:
  std::vector<twist::ed::std::thread> threads_;
  IntrusiveUnboundedBlockingQueue<task::TaskBase> task_queue_;
  size_t n_threads_{0};
  bool stopped_{false};
};

}  // namespace exe::sched
