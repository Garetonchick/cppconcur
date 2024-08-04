#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>

#include <exe/fiber/sync/strand.hpp>
#include <exe/fiber/sync/mutex.hpp>
#include <exe/fiber/sync/wait_group.hpp>

#include <exe/thread/wait_group.hpp>

#include <exe/util/defer.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

const size_t kFibers = 1'000;
const size_t kCritical = 1'000;

void StrandTest() {
  sched::ThreadPool scheduler{4};

  thread::WaitGroup example;
  fiber::Strand strand;
  size_t counter = 0;

  for(size_t i = 0; i < kFibers; ++i) {
    example.Add(1);
    fiber::Go(scheduler, [&example, &strand, &counter] {
      for (size_t j = 0; j < kCritical; ++j) {
        strand.Combine([&counter](){
          ++counter;
        });
      }
      example.Done();
    });
  }

  scheduler.Start();
  example.Wait();
  scheduler.Stop();
}

void MutexTest() {
  sched::ThreadPool scheduler{4};

  thread::WaitGroup example;
  fiber::Mutex mu;
  size_t counter = 0;

  for(size_t i = 0; i < kFibers; ++i) {
    example.Add(1);
    fiber::Go(scheduler, [&example, &mu, &counter] {
      for (size_t j = 0; j < kCritical; ++j) {
        mu.Lock();
        ++counter;
        mu.Unlock();
      }
      example.Done();
    });
  }

  scheduler.Start();
  example.Wait();
  scheduler.Stop();
}

int main() {
//  StrandTest();
  MutexTest();

  return 0;
}
