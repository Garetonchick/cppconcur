#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/event.hpp>

#include <exe/thread/wait_group.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(Event) {
  TWIST_STRESS_TEST(Storage, 5s) {
    sched::ThreadPool scheduler{3};
    scheduler.Start();

    course::test::TimeBudget time_budget;

    size_t iter_count = 0;

    while (time_budget) {
      ++iter_count;

      thread::WaitGroup iter;

      iter.Add(1);

      fiber::Go(scheduler, [&iter] {
        auto* event = new fiber::Event{};

        fiber::Go([event] {
          event->Fire();
        });

        event->Wait();
        delete event;

        iter.Done();
      });

      iter.Wait();
    }

    scheduler.Stop();

    fmt::println("# iterations = {}", iter_count);
  }
}

RUN_ALL_TESTS();
