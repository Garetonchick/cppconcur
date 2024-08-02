#include <wheels/test/framework.hpp>

#include <exe/sched/run_loop.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/event.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(Event) {
  SIMPLE_TEST(OneWaiter) {
    sched::RunLoop scheduler;

    static const std::string kMessage = "Hello";

    fiber::Event event;
    std::string data;
    bool ok = false;

    fiber::Go(scheduler, [&] {
      event.Wait();
      ASSERT_EQ(data, kMessage);
      ok = true;
    });

    fiber::Go(scheduler, [&] {
      data = kMessage;
      event.Fire();
    });

    scheduler.Run();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(MultipleWaiters) {
    sched::RunLoop scheduler;

    fiber::Event event;
    int work = 0;
    size_t waiters = 0;

    static const size_t kWaiters = 7;

    for (size_t i = 0; i < kWaiters; ++i) {
      fiber::Go(scheduler, [&] {
        event.Wait();
        ASSERT_EQ(work, 1);
        ++waiters;
      });
    }

    fiber::Go(scheduler, [&] {
      ++work;
      event.Fire();
    });

    scheduler.Run();

    ASSERT_EQ(waiters, kWaiters);
  }

  SIMPLE_TEST(SuspendFiber) {
    sched::RunLoop scheduler;

    fiber::Event event;
    bool ok = false;

    fiber::Go(scheduler, [&] {
      event.Wait();
      ok = true;
    });

    {
      size_t tasks = scheduler.Run();
      ASSERT_LE(tasks, 7);
    }

    fiber::Go(scheduler, [&] {
      event.Fire();
    });

    scheduler.Run();

    ASSERT_TRUE(ok);
  }
}

RUN_ALL_TESTS()
