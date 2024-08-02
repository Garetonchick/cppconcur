#include <wheels/test/framework.hpp>

#include <exe/sched/run_loop.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>
#include <exe/fiber/sync/event.hpp>
#include <exe/fiber/sync/mutex.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(Mutex) {
  SIMPLE_TEST(Lock) {
    sched::RunLoop scheduler;

    fiber::Mutex mutex;
    size_t cs = 0;

    fiber::Go(scheduler, [&] {
      mutex.Lock();
      ++cs;
      mutex.Unlock();

      mutex.Lock();
      ++cs;
      mutex.Unlock();
    });

    scheduler.Run();

    ASSERT_EQ(cs, 2);
  }

  SIMPLE_TEST(TryLock) {
    sched::RunLoop scheduler;

    fiber::Go(scheduler, [&] {
      fiber::Mutex mutex;

      {
        ASSERT_TRUE(mutex.TryLock());
        mutex.Unlock();
      }

      {
        mutex.Lock();
        mutex.Unlock();
      }

      ASSERT_TRUE(mutex.TryLock());

      bool join = false;

      fiber::Go([&] {
        ASSERT_FALSE(mutex.TryLock());
        join = true;
      });

      while (!join) {
        fiber::Yield();
      }

      mutex.Unlock();
    });

    scheduler.Run();
  }

  SIMPLE_TEST(Lockable) {
    sched::RunLoop scheduler;

    fiber::Go(scheduler, [&] {
      fiber::Mutex mutex;

      {
        std::lock_guard guard{mutex};
      }

      {
        std::unique_lock lock{mutex, std::try_to_lock};
        ASSERT_TRUE(lock.owns_lock());
      }
    });

    scheduler.Run();
  }

  SIMPLE_TEST(LockManyTimes) {
    sched::RunLoop scheduler;

    fiber::Mutex mutex;
    size_t cs = 0;

    fiber::Go(scheduler, [&] {
      for (size_t j = 0; j < 11; ++j) {
        std::lock_guard guard(mutex);
        ++cs;
      }
    });

    scheduler.Run();

    ASSERT_EQ(cs, 11);
  }

  SIMPLE_TEST(Counter) {
    sched::RunLoop scheduler;

    fiber::Mutex mutex;
    size_t cs = 0;

    static const size_t kFibers = 5;
    static const size_t kSectionsPerFiber = 5;

    for (size_t i = 0; i < kFibers; ++i) {
      fiber::Go(scheduler, [&] {
        for (size_t j = 0; j < kSectionsPerFiber; ++j) {
          std::lock_guard guard(mutex);

          ++cs;
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

    fiber::Mutex mutex;
    fiber::Event unlock;

    fiber::Go(scheduler, [&] {
      mutex.Lock();
      unlock.Wait();
      mutex.Unlock();
    });

    bool cs = false;

    fiber::Go(scheduler, [&] {
      mutex.Lock();
      cs = true;
      mutex.Unlock();
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

  SIMPLE_TEST(Fifo) {
    sched::RunLoop scheduler;

    fiber::Mutex mutex;

    fiber::Go(scheduler, [&] {
      mutex.Lock();

      for (size_t i = 0; i < 1024; ++i) {
        fiber::Yield();
      }

      mutex.Unlock();
    });

    const size_t kWaiters = 16;

    scheduler.RunAtMost(7);  // Lock mutex

    size_t next_waiter = 0;

    for (size_t i = 0; i < kWaiters; ++i) {
      fiber::Go(scheduler, [&, i] {
        mutex.Lock();

        ASSERT_EQ(next_waiter, i);
        ++next_waiter;

        mutex.Unlock();
      });

      // mutex.Lock() -> wait queue
      scheduler.RunAtMost(7);
    }

    scheduler.Run();
  }
}

RUN_ALL_TESTS()
