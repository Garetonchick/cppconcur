#include "thread_pool.hpp"

#include <iostream>
#include <cstdlib>  // std::abort
#include <twist/ed/static/thread_local/ptr.hpp>
#include <twist/ed/static/thread_local/var.hpp>
#include <twist/ed/static/var.hpp>
#include <twist/ed/wait/futex.hpp>

#include <exe/util/defer.hpp>

namespace exe::sched::tp::fast {

struct ThreadIndexOffset : twist::ed::std::atomic<size_t> {
  ThreadIndexOffset()
      : twist::ed::std::atomic<size_t>(0) {
  }
};

TWISTED_STATIC_THREAD_LOCAL_PTR(ThreadPool, current_threadpool);
TWISTED_STATIC_THREAD_LOCAL(size_t, current_thread_idx);
TWISTED_STATIC(ThreadIndexOffset, thread_index_offset);

ThreadPool::ThreadPool(size_t threads)
    : threads_(threads),
      coordinator_(threads) {
  thread_idx_offset_ = thread_index_offset->fetch_add(threads) + 1;
  //  std::cout << "ThreadPool constructor" << std::endl;
}

void ThreadPool::Start() {
  for (size_t i = 0; i < threads_; ++i) {
    workers_.emplace_back(*this, i);
  }
  for (auto& worker : workers_) {
    worker.Start();
  }
}

ThreadPool::~ThreadPool() {
}

void ThreadPool::Submit(task::TaskBase* task, task::SchedulerHint hint) {
  Defer maybe_wakeup([this]() {
    if (n_spinning_.load() == 0) {
      //      std::cout << "n_spinning=0, trying to wake someone..." <<
      //      std::endl;
      auto waiter = PopWaiter();
      if (waiter) {
        workers_[*waiter].Wake();
        //        std::cout << "wake " << *waiter << std::endl;
      } else {
        //        std::cout << "no one to wake " << std::endl;
      }
      //    } else {
      //      std::cout << "have spinning " << std::endl;
    }
  });

  if (*current_thread_idx < thread_idx_offset_ ||
      *current_thread_idx >= thread_idx_offset_ + threads_) {
    global_tasks_.PushOne(task);
    return;
  }

  size_t worker_idx = *current_thread_idx - thread_idx_offset_;
  workers_[worker_idx].Push(task, hint);
}

void ThreadPool::TryWakeOneWorker() {
  for (size_t i = 0; i < 3; ++i) {
    auto waiter = PopWaiter();
    if (waiter) {
      workers_[*waiter].Wake();
    }
  }
}

void ThreadPool::Stop() {
  ready_to_stop_.store(true);
  if (n_parked_.load() != threads_) {
    twist::ed::futex::Wait(stopped_, 0);
  } else {
    //    std::cout << "Skip waiting on futex" << std::endl;
    stopped_.store(1);
  }

  for (auto& worker : workers_) {
    worker.StoppingWake();
  }

  for (auto& worker : workers_) {
    worker.Join();
  }
}

PoolMetrics ThreadPool::Metrics() const {
  std::abort();  // Mop implemented
}

ThreadPool* ThreadPool::Current() {
  return current_threadpool;
}

void ThreadPool::SetCurrentThreadPool() {
  current_threadpool = this;
}

void ThreadPool::SetCurrentThreadIdx(size_t worker_idx) {
  *current_thread_idx = thread_idx_offset_ + worker_idx;
}

void ThreadPool::SignalStop() {
  if (ready_to_stop_.load()) {
    auto wake_key = twist::ed::futex::PrepareWake(stopped_);
    stopped_.store(1);
    twist::ed::futex::WakeOne(wake_key);
    //    std::cout << "Signaled stop" << std::endl;
  }
}

// TODO: lock-free
void ThreadPool::PushWaiter(size_t idx) {
  std::lock_guard guard(waiters_lock_);
  waiters_.push_back(idx);
}

std::optional<size_t> ThreadPool::PopWaiter() {
  std::lock_guard guard(waiters_lock_);
  if (waiters_.empty()) {
    return std::nullopt;
  }
  size_t waiter = waiters_.front();
  waiters_.pop_front();
  return waiter;
}

}  // namespace exe::sched::tp::fast
