#pragma once

#include <exe/sched/task/scheduler.hpp>
#include <exe/thread/spinlock.hpp>

#include "queues/global_queue.hpp"
#include "worker.hpp"
#include "coordinator.hpp"
#include "metrics.hpp"

// std::random_device
#include <twist/ed/std/random.hpp>
#include <twist/ed/std/condition_variable.hpp>

#include <deque>

namespace exe::sched::tp::fast {

// Scalable work-stealing scheduler for
// fibers, stackless coroutines and futures

class ThreadPool : public task::IScheduler {
  friend class Worker;

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
  void Submit(task::TaskBase*, task::SchedulerHint) override;

  void Stop();

  // After Stop
  PoolMetrics Metrics() const;

  static ThreadPool* Current();

 private:
  void SetCurrentThreadPool();
  void SetCurrentThreadIdx(size_t idx);
  void PushWaiter(size_t idx);
  std::optional<size_t> PopWaiter();
  void SignalStop();
  void TryWakeOneWorker();

 private:
  twist::ed::std::atomic_uint32_t stopped_{0};
  twist::ed::std::atomic_bool ready_to_stop_{false};
  const size_t threads_;
  std::deque<Worker> workers_;
  Coordinator coordinator_;
  GlobalQueue global_tasks_;
  PoolMetrics metrics_;
  twist::ed::std::random_device random_device_;
  size_t thread_idx_offset_{0};

  // parking
  twist::ed::std::atomic<size_t> n_spinning_{0};
  twist::ed::std::atomic<size_t> n_parked_{0};
  twist::ed::std::atomic_bool last_circle_{false};
  twist::ed::std::atomic<size_t> n_last_circlers_{0};
  thread::SpinLock waiters_lock_;
  std::deque<size_t> waiters_;
  twist::ed::std::condition_variable not_all_waiting_;
};

}  // namespace exe::sched::tp::fast
