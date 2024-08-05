#include <exe/sched/thread_pool.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sched/yield.hpp>
#include <exe/coro/sync/mutex.hpp>
#include <exe/coro/sync/wait_group.hpp>

#include <exe/thread/wait_group.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

int main() {
  sched::ThreadPool scheduler{4};
  scheduler.Start();

  thread::WaitGroup example;

  auto coordinator = [&] -> coro::Go {
    co_await coro::JumpTo(scheduler);

    coro::Mutex mutex;
    size_t cs = 0;

    coro::WaitGroup wg;

    auto contender = [&] -> coro::Go {
      co_await coro::JumpTo(scheduler);

      for (size_t i = 0; i < 100'000; ++i) {
        auto guard = co_await mutex.ScopedLock();
        ++cs;
      }

      wg.Done();
    };

    for (size_t i = 0; i < 17; ++i) {
      wg.Add(1);
      contender();
    }

    co_await wg.Wait();

    fmt::println("#cs = {}", cs);

    example.Done();
  };

  example.Add(1);
  coordinator();
  example.Wait();

  scheduler.Stop();
}
