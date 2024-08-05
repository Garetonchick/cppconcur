#include <exe/sched/run_loop.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sync/event.hpp>
#include <exe/coro/sync/mutex.hpp>

#include <wheels/test/framework.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(Mutex) {
  SIMPLE_TEST(Lock) {
    sched::RunLoop loop;

    bool cs = false;

    auto locker = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      coro::Mutex mutex;

      {
        auto guard = co_await mutex.ScopedLock();
        cs = true;
      }
    };

    locker();

    loop.Run();

    ASSERT_TRUE(cs);
  }

  SIMPLE_TEST(LockLoop) {
    sched::RunLoop loop;

    coro::Mutex mutex;
    size_t cs = 0;

    auto locker = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      for (size_t j = 0; j < 11; ++j) {
        auto guard = co_await mutex.ScopedLock();
        ++cs;
      }
    };

    locker();

    loop.Run();

    ASSERT_EQ(cs, 11);
  }

  SIMPLE_TEST(LockWait) {
    sched::RunLoop loop;

    coro::Mutex mutex;
    coro::Event unlock;

    auto fst = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      {
        auto guard = co_await mutex.ScopedLock();

        co_await unlock.Wait();
      }
    };

    fst();

    bool cs = false;

    auto snd = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      {
        auto guard = co_await mutex.ScopedLock();

        cs = true;
      }
    };

    snd();

    {
      size_t tasks = loop.Run();
      ASSERT_LE(tasks, 17);
      ASSERT_FALSE(cs);
    }

    auto resume = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      unlock.Fire();
    };

    resume();

    loop.Run();

    ASSERT_TRUE(cs);
  }

  SIMPLE_TEST(Waiters) {
    sched::RunLoop loop;

    coro::Mutex mutex;
    coro::Event unlock;

    auto locker = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      {
        auto guard = co_await mutex.ScopedLock();

        co_await unlock.Wait();
      }
    };

    locker();

    // Acquire mutex ownership
    loop.RunAtMost(7);

    size_t cs = 0;

    auto waiter = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      {
        auto guard = co_await mutex.ScopedLock();
        ++cs;
      }
    };

    const size_t kWaiters = 7;

    for (size_t i = 0; i < kWaiters; ++i) {
      waiter();
    }

    {
      size_t tasks = loop.Run();
      ASSERT_LE(tasks, 17);
      ASSERT_EQ(cs, 0);
    }

    auto resume = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      unlock.Fire();
    };

    resume();

    loop.Run();

    ASSERT_EQ(cs, kWaiters);
  }

  SIMPLE_TEST(Fifo) {
    sched::RunLoop loop;

    coro::Mutex mutex;
    coro::Event unlock;

    auto locker = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      auto guard = co_await mutex.ScopedLock();

      co_await unlock.Wait();
    };

    locker();

    loop.RunAtMost(7);

    const size_t kWaiters = 16;

    size_t next_waiter = 0;

    auto waiter = [&](size_t i) -> coro::Go {
      co_await coro::JumpTo(loop);

      auto guard = co_await mutex.ScopedLock();

      ASSERT_EQ(next_waiter, i);
      ++next_waiter;
    };

    for (size_t i = 0; i < kWaiters; ++i) {
      waiter(i);

      // mutex.ScopedLock() -> wait queue
      loop.RunAtMost(7);
    }

    auto resume = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      unlock.Fire();
    };

    resume();

    loop.Run();
  }
}

RUN_ALL_TESTS()
