#include "worker.hpp"
#include "thread_pool.hpp"

#include <twist/ed/static/thread_local/ptr.hpp>
#include <twist/ed/static/thread_local/var.hpp>
#include <wheels/core/assert.hpp>
#include <twist/ed/wait/futex.hpp>

#include <array>
#include <chrono>
#include <algorithm>
#include <iostream>

namespace exe::sched::tp::fast {

using TaskBuffer = std::array<task::TaskBase*, Worker::kLocalQueueCapacity>;

TWISTED_STATIC_THREAD_LOCAL(TaskBuffer, task_buffer);

Worker::Worker(ThreadPool& host, size_t index)
    : host_(host),
      index_(index) {
  stealing_order_.resize(host_.threads_ - 1);
}

void Worker::Start() {
  thread_.emplace([this] {
    Work();
  });
}

void Worker::Join() {
  thread_->join();
}

void Worker::Push(task::TaskBase* task, task::SchedulerHint hint) {
  if (hint == task::SchedulerHint::Yield) {
    host_.global_tasks_.PushOne(task);
    return;
  }

  if (hint == task::SchedulerHint::Next) {
    std::swap(lifo_slot_, task);
    if (task == nullptr) {
      return;
    }
  }

  if (local_tasks_.TryPush(task)) {
    return;
  }
  // local queue is full, lay off half of the tasks to global queue
  OffloadTasksToGlobalQueue(task);
}

size_t Worker::StealTasks(std::span<task::TaskBase*> out_buffer) {
  size_t n_grab = local_tasks_.SizeEstimate();
  n_grab = std::min((n_grab + 1) / 2,
                    out_buffer.size());  // steal no more than half of tasks
  if (n_grab == 0) {
    return 0;
  }
  return local_tasks_.Grab(
      std::span(out_buffer.begin(), out_buffer.begin() + n_grab));
}

task::TaskBase* Worker::PickTask() {
  // Poll in order:
  // * [%61] global queue
  // * LIFO slot
  // * local queue
  // * global queue
  // * work stealing
  // then
  //   park worker

  ++iter_;
  auto* task = TryPickTask();
  if (task != nullptr) {
    return task;
  }
  return TryPickTaskBeforePark();
}

void Worker::StoppingWake() {
  //  std::cout << "Stopping wake for worker " << index_ << std::endl;
  worker_stop_.store(true);
  Wake();
}

void Worker::Wake() {
  auto wake_key = twist::ed::futex::PrepareWake(wakeups_);
  wakeups_.fetch_add(1);
  twist::ed::futex::WakeOne(wake_key);
}

void Worker::Work() {
  host_.SetCurrentThreadPool();
  host_.SetCurrentThreadIdx(index_);

  while (!worker_stop_.load()) {
    task::TaskBase* next = PickTask();
    if (next != nullptr) {
      //      std::cout << "worker " << index_ << std::endl;
      next->Run();
    }
  }
  //  std::cout << "Exit worker " << index_ << std::endl;
}

task::TaskBase* Worker::TryPickTask() {
  //  std::cout << "try pick task" << std::endl;
  task::TaskBase* task = nullptr;
  if (iter_ % 61 == 0) {
    task = TryPickTaskFromGlobalQueue();
    if (task != nullptr) {
      return task;
    }
  }

  // TODO: limit
  task = TryPickTaskFromLifoSlot();
  if (task != nullptr) {
    return task;
  }

  task = TryPickTaskFromLocalQueue();
  if (task != nullptr) {
    return task;
  }

  task = TryPickTaskFromGlobalQueue();
  if (task != nullptr) {
    return task;
  }

  return nullptr;
}

task::TaskBase* Worker::TryPickTaskBeforePark() {
  host_.n_spinning_.fetch_add(1);
  auto task = TryStealTasks();
  if (task != nullptr) {
    host_.n_spinning_.fetch_add(-1);
    host_.TryWakeOneWorker();
    return task;
  }

  // TODO: maybe a bit of actual steal spinning before park?

  size_t old_wakeups = wakeups_.load();
  host_.PushWaiter(index_);  // pessimistic push
  host_.n_spinning_.fetch_add(-1);

  task = TryPickTaskFromGlobalQueue();
  if (task != nullptr) {
    host_.TryWakeOneWorker();
    host_.TryWakeOneWorker();
    return task;
  }

  // final attempt
  task = TryStealTasks();
  if (task != nullptr) {
    // TODO: wake up one more worker?
    host_.TryWakeOneWorker();
    host_.TryWakeOneWorker();
    return task;
  }

  // park

  size_t old_n_parked = host_.n_parked_.fetch_add(1);
  ++old_n_parked;

  if (old_n_parked == host_.threads_ && host_.ready_to_stop_.load()) {
    //    std::cout << "Stopping all" << std::endl;
    worker_stop_.store(true);
    host_.SignalStop();
    for (auto& worker : host_.workers_) {
      worker.StoppingWake();
    }
    return nullptr;
  }

  if (host_.stopped_.load() != 0u) {
    worker_stop_.store(true);
    return nullptr;
  }

  twist::ed::futex::Wait(wakeups_, old_wakeups);
  host_.n_parked_.fetch_add(-1);

  // now recheck tasks
  return TryPickTask();
  //  task = TryPickTaskFromGlobalQueue();
  //  if (task != nullptr) {
  //    //    std::cout << "final global queue pick try success" << std::endl;
  //    return task;
  //  }

  //  return nullptr;
}

task::TaskBase* Worker::TryPickTaskFromLifoSlot() {
  return std::exchange(lifo_slot_, nullptr);
}

task::TaskBase* Worker::TryStealTasks() {
  if (host_.threads_ <= 1) {
    return nullptr;
  }

  std::iota(stealing_order_.begin(), stealing_order_.end(), 0);
  std::shuffle(stealing_order_.begin(), stealing_order_.end(), twister_);

  auto try_steal_from = [this](size_t victim) -> task::TaskBase* {
    auto steal_span = std::span(task_buffer->begin(),
                                task_buffer->begin() + kLocalQueueCapacity / 2);
    size_t n_stolen = host_.workers_[victim].StealTasks(steal_span);
    if (n_stolen == 0) {
      return nullptr;
    }
    auto stolen_task = steal_span[n_stolen - 1];

    for (size_t i = 0; i < n_stolen - 1; ++i) {
      bool ok = local_tasks_.TryPush(steal_span[i]);
      if (!ok) {
        std::cerr << "Failed push to local_tasks_" << std::endl;
        std::abort();
      }
    }

    return stolen_task;
  };

  for (size_t victim : stealing_order_) {
    if (victim == index_) {
      victim = host_.threads_ - 1;
    }
    auto task = try_steal_from(victim);
    if (task != nullptr) {
      return task;
    }
  }
  return nullptr;
}

void Worker::OffloadTasksToGlobalQueue(task::TaskBase* overflowed_task) {
  auto grab_span = std::span(task_buffer->begin(),
                             task_buffer->begin() + kLocalQueueCapacity / 2);
  size_t n_grab = local_tasks_.Grab(grab_span);

  (*task_buffer)[n_grab] = overflowed_task;
  host_.global_tasks_.PushMany(
      std::span(task_buffer->begin(), task_buffer->begin() + n_grab + 1));
}

task::TaskBase* Worker::TryPickTaskFromGlobalQueue() {
  // TODO: Grab more
  return host_.global_tasks_.TryPopOne();
}

task::TaskBase* Worker::TryGrabTasksFromGlobalQueue() {
  std::abort();  // Mop implemented
}

task::TaskBase* Worker::TryPickTaskFromLocalQueue() {
  return local_tasks_.TryPop();
}

}  // namespace exe::sched::tp::fast
