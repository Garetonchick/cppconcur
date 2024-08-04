#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/event.hpp>

#include <exe/thread/wait_group.hpp>

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

      thread::WaitGroup iter;

      twist::assist::Shared<int> data = 0;
      fiber::Event event;

      iter.Add(1);

      fiber::Go(scheduler, [&] {
        data.Write(1);
        event.Fire();

        iter.Done();
      });

      iter.Add(1);

      fiber::Go(scheduler, [&] {
        event.Wait();

        int d = data.Read();
        TWIST_TEST_ASSERT(d == 1, "Missing work");

        iter.Done();
      });

      iter.Wait();
    }

    scheduler.Stop();

    fmt::println("# iterations = {}", iter_count);
  }
}

RUN_ALL_TESTS();
