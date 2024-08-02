#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/mutex.hpp>

#include <exe/thread/wait_group.hpp>

#include <twist/test/plate.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(StressMutex) {
  TWIST_STRESS_TEST(StackOverflow, 10s) {
    // Parameters
    const size_t kFibers = 4096;
    const size_t kThreads = 3;
    const size_t kSmallStackSize = 8 * 1024;  // 8KB

    sched::ThreadPool scheduler{kThreads};
    scheduler.Start();

    thread::WaitGroup wg;

    fiber::Mutex mutex;
    twist::test::Plate plate;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);

      fiber::Go(scheduler, [&] {
        course::test::TimeBudget time_budget{5s};

        while (time_budget) {
          mutex.Lock();
          plate.Access();
          mutex.Unlock();
        }

        wg.Done();
      }, {.stack_size = kSmallStackSize});
    }

    wg.Wait();

    fmt::println("# cs = {}", plate.AccessCount());

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
