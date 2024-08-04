#include <wheels/test/framework.hpp>

#include <exe/sched/run_loop.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>
#include <exe/fiber/sync/event.hpp>
#include <exe/fiber/sync/strand.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(Strand) {
  SIMPLE_TEST(Lock) {
    sched::RunLoop scheduler;

    fiber::Strand mutex;
    size_t cs = 0;

    fiber::Go(scheduler, [&] {
      mutex.Combine([&] {
        ++cs;
      });

      ASSERT_EQ(cs, 1);

      mutex.Combine([&] {
        ++cs;
      });

      ASSERT_EQ(cs, 2);
    });

    scheduler.Run();

    ASSERT_EQ(cs, 2);
  }

  SIMPLE_TEST(LockManyTimes) {
    sched::RunLoop scheduler;

    fiber::Strand mutex;
    size_t cs = 0;

    fiber::Go(scheduler, [&] {
      for (size_t j = 0; j < 11; ++j) {
        mutex.Combine([&] {
          ++cs;
        });
      }
    });

    scheduler.Run();

    ASSERT_EQ(cs, 11);
  }

  SIMPLE_TEST(Counter) {
    sched::RunLoop scheduler;

    fiber::Strand mutex;
    size_t cs = 0;

    static const size_t kFibers = 5;
    static const size_t kSectionsPerFiber = 5;

    for (size_t i = 0; i < kFibers; ++i) {
      fiber::Go(scheduler, [&] {
        for (size_t j = 0; j < kSectionsPerFiber; ++j) {
          mutex.Combine([&] {
            ++cs;
          });
          fiber::Yield();
        }
      });
    }

    scheduler.Run();

    fmt::println("# cs = {}, expected = {}",
                 cs, kFibers * kSectionsPerFiber);

    ASSERT_EQ(cs, kFibers * kSectionsPerFiber);
  }

  SIMPLE_TEST(SuspendFiber) {
    sched::RunLoop scheduler;

    fiber::Strand mutex;
    fiber::Event unlock;

    fiber::Go(scheduler, [&] {
      mutex.Combine([&] {
        unlock.Wait();  // Illegal?
      });
    });

    bool cs = false;

    fiber::Go(scheduler, [&] {
      mutex.Combine([&] {
        cs = true;
      });
    });

    {
      size_t tasks = scheduler.Run();
      ASSERT_LE(tasks, 17);
      ASSERT_FALSE(cs);
    }

    fiber::Go(scheduler, [&] {
      unlock.Fire();
    });

    scheduler.Run();

    ASSERT_TRUE(cs);
  }
}

RUN_ALL_TESTS()
