#include "thread_pool.hpp"

#include <twist/ed/static/thread_local/ptr.hpp>
#include <cassert>

namespace exe::sched::tp::compute {

TWISTED_STATIC_THREAD_LOCAL_PTR(ThreadPool, current_threadpool);

ThreadPool::ThreadPool(size_t threads)
    : n_threads_(threads) {
}

void ThreadPool::Start() {
  threads_.reserve(n_threads_);

  auto worker = [this]() {
    current_threadpool = this;
    try {
      for (;;) {
        auto task_or_empty = task_queue_.Pop();
        if (task_or_empty == std::nullopt) {
          return;
        }
        task_or_empty.value()->Run();
      }
    } catch (...) {
      std::abort();
    }
  };

  for (size_t i = 0; i < n_threads_; ++i) {
    threads_.emplace_back(worker);
  }
}

ThreadPool::~ThreadPool() {
  assert(stopped_);
}

void ThreadPool::Submit(task::TaskBase* task, task::SchedulerHint) {
  task_queue_.Push(task);
}

ThreadPool* ThreadPool::Current() {
  return current_threadpool;
}

void ThreadPool::Stop() {
  task_queue_.Close();
  for (auto& thread : threads_) {
    thread.join();
  }
  stopped_ = true;
}

}  // namespace exe::sched::tp::compute
