#include <exe/sched/thread_pool.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sched/yield.hpp>
#include <exe/coro/sync/wait_group.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/test/assert.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(StressWaitGroup) {
  TWIST_STRESS_TEST(Load, 5s) {
    sched::ThreadPool scheduler{3};
    scheduler.Start();

    thread::WaitGroup test;

    auto tester = [&] -> coro::Go {
      co_await coro::JumpTo(scheduler);

      course::test::TimeBudget time_budget;

      coro::WaitGroup wg;

      size_t iter_count = 0;

      while (time_budget) {
        ++iter_count;

        size_t workers = 3 + iter_count % 5;

        twist::ed::std::atomic_size_t work = 0;

        auto worker = [&] -> coro::Go {
          co_await coro::JumpTo(scheduler);

          for (size_t j = 0; j < 7; ++j) {
            co_await coro::Yield();
          }
          work.fetch_add(1);
          wg.Done();
        };

        for (size_t i = 0; i < workers; ++i) {
          wg.Add(1);
          worker();
        }

        co_await wg.Wait();

        TWIST_TEST_ASSERT(work.load() == workers, "Missing work");
      }

      fmt::println("# groups = {}", iter_count);

      test.Done();
    };

    test.Add(1);
    tester();
    test.Wait();

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
