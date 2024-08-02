#include <wheels/test/framework.hpp>

#include <exe/sched/run_loop.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/wait_group.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(WaitGroup) {
  SIMPLE_TEST(OneWaiter) {
    sched::RunLoop scheduler;

    fiber::WaitGroup wg;
    size_t work = 0;
    bool ok = false;

    const size_t kWorkers = 3;

    for (size_t i = 0; i < kWorkers; ++i) {
      wg.Add(1);
      fiber::Go(scheduler, [&] {
        ++work;
        wg.Done();
      });
    }

    fiber::Go(scheduler, [&] {
      wg.Wait();
      ASSERT_EQ(work, kWorkers);
      ok = true;
    });

    scheduler.Run();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(MultipleWaiters) {
    sched::RunLoop scheduler;

    fiber::WaitGroup wg;

    size_t work = 0;
    size_t acks = 0;

    const size_t kWorkers = 3;
    const size_t kWaiters = 4;

    for (size_t i = 0; i < kWorkers; ++i) {
      wg.Add(1);
      fiber::Go(scheduler, [&] {
        ++work;
        wg.Done();
      });
    }

    for (size_t i = 0; i < kWaiters; ++i) {
      fiber::Go(scheduler, [&] {
        wg.Wait();
        ASSERT_EQ(work, kWorkers);
        ++acks;
      });
    }

    scheduler.Run();

    ASSERT_EQ(acks, kWaiters);
  }

  SIMPLE_TEST(SuspendFiber) {
    sched::RunLoop scheduler;

    fiber::WaitGroup wg;
    size_t work = 0;
    bool ok = false;

    const size_t kWorkers = 3;

    wg.Add(kWorkers);

    fiber::Go(scheduler, [&] {
      wg.Wait();
      ASSERT_EQ(work, kWorkers);
      ok = true;
    });

    {
      size_t tasks = scheduler.Run();
      ASSERT_LE(tasks, 7);
    }

    for (size_t i = 0; i < kWorkers; ++i) {
      fiber::Go(scheduler, [&] {
        ++work;
        wg.Done();
      });
    }

    scheduler.Run();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(Cyclic) {
    sched::RunLoop scheduler;

    fiber::WaitGroup wg;

    const size_t kIters = 3;

    for (size_t k = 0; k < kIters; ++k) {
      const size_t kWork = 5;

      size_t work = 0;

      for (size_t i = 0; i < kWork; ++i) {
        wg.Add(1);
        fiber::Go(scheduler, [&] {
          ++work;
          wg.Done();
        });

        fiber::Go(scheduler, [&] {
          wg.Wait();
          ASSERT_EQ(work, kWork);
        });
      }

      scheduler.Run();
    }
  }
}

RUN_ALL_TESTS()
