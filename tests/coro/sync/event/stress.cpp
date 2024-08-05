#include <exe/sched/thread_pool.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sync/event.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/assist/shared.hpp>
#include <twist/test/assert.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(StressEvent) {
  TWIST_STRESS_TEST(MessagePassing, 5s) {
    sched::ThreadPool scheduler{3};
    scheduler.Start();

    course::test::TimeBudget time_budget;

    size_t iter_count = 0;

    while (time_budget) {
      ++iter_count;

      thread::WaitGroup test_iter;

      twist::assist::Shared<int> data = 0;
      coro::Event event;

      test_iter.Add(2);

      auto worker = [&] -> coro::Go {
        co_await coro::JumpTo(scheduler);

        data.Write(1);
        event.Fire();

        test_iter.Done();
      };

      auto waiter = [&] -> coro::Go {
        co_await coro::JumpTo(scheduler);

        co_await event.Wait();

        int d = data.Read();
        TWIST_TEST_ASSERT(d == 1, "Missing message");

        test_iter.Done();
      };

      waiter();
      worker();

      test_iter.Wait();
    }

    scheduler.Stop();

    fmt::println("# iterations = {}", iter_count);
  }
}

RUN_ALL_TESTS();
