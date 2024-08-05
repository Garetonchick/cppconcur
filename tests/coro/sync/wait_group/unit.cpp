#include <exe/sched/run_loop.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sync/wait_group.hpp>

#include <wheels/test/framework.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(WaitGroup) {
  SIMPLE_TEST(WaitReady) {
    sched::RunLoop loop;

    coro::WaitGroup wg;
    size_t work = 0;
    bool ok = false;

    const size_t kWorkers = 3;

    auto worker = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      ++work;
      wg.Done();
    };

    for (size_t i = 0; i < kWorkers; ++i) {
      wg.Add(1);
      worker();
    }

    auto waiter = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      co_await wg.Wait();
      ASSERT_EQ(work, kWorkers);
      ok = true;
    };

    waiter();

    loop.Run();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(WaitSuspend) {
    sched::RunLoop loop;

    coro::WaitGroup wg;
    size_t work = 0;
    bool wait = false;

    const size_t kWorkers = 3;

    wg.Add(kWorkers);

    auto waiter = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      co_await wg.Wait();
      ASSERT_EQ(work, kWorkers);
      wait = true;
    };

    waiter();

    {
      size_t tasks = loop.Run();
      ASSERT_LE(tasks, 7);
    }

    auto worker = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      ++work;
      wg.Done();
    };

    for (size_t i = 0; i < kWorkers; ++i) {
      worker();
    }

    loop.Run();

    ASSERT_TRUE(wait);
  }

  SIMPLE_TEST(Waiters) {
    sched::RunLoop loop;

    coro::WaitGroup wg;

    size_t work = 0;
    size_t acks = 0;

    const size_t kWorkers = 3;
    const size_t kWaiters = 4;

    auto waiter = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      co_await wg.Wait();
      ASSERT_EQ(work, kWorkers);
      ++acks;
    };

    for (size_t i = 0; i < kWaiters; ++i) {
      waiter();
    }

    auto worker = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      ++work;
      wg.Done();
    };

    for (size_t i = 0; i < kWorkers; ++i) {
      wg.Add(1);
      worker();
    }

    loop.Run();

    ASSERT_EQ(acks, kWaiters);
  }

  SIMPLE_TEST(Cyclic) {
    sched::RunLoop loop;

    coro::WaitGroup wg;

    const size_t kIters = 3;

    for (size_t k = 0; k < kIters; ++k) {
      const size_t kWork = 5;

      size_t work = 0;

      auto waiter = [&] -> coro::Go {
        co_await coro::JumpTo(loop);

        co_await wg.Wait();
        ASSERT_EQ(work, kWork);
      };

      auto worker = [&] -> coro::Go {
        co_await coro::JumpTo(loop);

        ++work;
        wg.Done();
      };

      for (size_t i = 0; i < kWork; ++i) {
        wg.Add(1);
        worker();
        waiter();
      }

      loop.Run();
    }
  }
}

RUN_ALL_TESTS()
