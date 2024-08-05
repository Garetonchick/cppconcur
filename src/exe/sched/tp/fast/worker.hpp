#pragma once

#include <exe/sched/task/task.hpp>
#include <exe/sched/task/hint.hpp>

#include "fwd.hpp"
#include "queues/work_stealing_queue.hpp"
#include "metrics.hpp"

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/std/thread.hpp>
#include <twist/build.hpp>

#include <cstdlib>
#include <optional>
#include <random>
#include <span>

namespace exe::sched::tp::fast {

class Worker {
 public:
  static constexpr size_t kLocalQueueCapacity =
      twist::build::kTwisted ? 7 : 256;

 public:
  Worker(ThreadPool& host, size_t index);

  void Start();
  void Join();

  // Single producer
  void Push(task::TaskBase*, task::SchedulerHint);

  // Steal from this worker
  size_t StealTasks(std::span<task::TaskBase*> out_buffer);

  // Wake parked worker
  void Wake();
  void StoppingWake();

  static Worker* Current();

  WorkerMetrics Metrics() const {
    return metrics_;
  }

  ThreadPool* Host() const {
    return &host_;
  }

  size_t GetIndex() const {
    return index_;
  }

 private:
  // Use in Push
  void PushToLifoSlot(task::TaskBase* task);
  void PushToLocalQueue(task::TaskBase* task);
  void OffloadTasksToGlobalQueue(task::TaskBase* overflowed_task);

  // Use in TryPickTask
  task::TaskBase* TryPickTaskFromLifoSlot();
  task::TaskBase* TryStealTasks();
  task::TaskBase* TryPickTaskFromGlobalQueue();
  task::TaskBase* TryPickTaskFromLocalQueue();
  task::TaskBase* TryGrabTasksFromGlobalQueue();

  // Use in PickTask
  task::TaskBase* TryPickTask();
  task::TaskBase* TryPickTaskBeforePark();

  task::TaskBase* PickTask();  // Or park thread

  // Run Loop
  void Work();

 private:
  ThreadPool& host_;
  const size_t index_;

  // Scheduling iteration
  size_t iter_ = 0;

  // Worker thread
  std::optional<twist::ed::std::thread> thread_;

  // Local queue
  WorkStealingQueue<kLocalQueueCapacity> local_tasks_;

  // LIFO slot
  task::TaskBase* lifo_slot_{nullptr};

  // Deterministic pseudo-randomness for work stealing
  std::mt19937_64 twister_;

  // Parking lot
  twist::ed::std::atomic<uint32_t> wakeups_{0};

  WorkerMetrics metrics_;

  std::vector<size_t> stealing_order_;
  twist::ed::std::atomic<bool> worker_stop_{false};
};

}  // namespace exe::sched::tp::fast
