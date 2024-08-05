#include <exe/sched/thread_pool.hpp>

#include <exe/coro/sched/jump.hpp>
#include <exe/coro/go.hpp>
#include <exe/coro/sync/wait_group.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(StressWaitGroup) {
  TWIST_STRESS_TEST(Storage, 5s) {
    sched::ThreadPool scheduler{3};
    scheduler.Start();

    course::test::TimeBudget time_budget;

    size_t iter_count = 0;

    while (time_budget) {
      ++iter_count;

      thread::WaitGroup test;

      auto test_case = [&] -> coro::Go {
        co_await coro::JumpTo(scheduler);

        auto* wg = new coro::WaitGroup{};

        auto producer = [&, wg] -> coro::Go {
          co_await coro::JumpTo(scheduler);

          wg->Done();
        };

        wg->Add(1);
        producer();

        co_await wg->Wait();
        delete wg;

        test.Done();
      };

      test.Add(1);
      test_case();
      test.Wait();
    }

    scheduler.Stop();

    fmt::println("# iterations = {}", iter_count);
  }
}

RUN_ALL_TESTS();
