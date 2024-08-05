#include <exe/sched/thread_pool.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sched/yield.hpp>
#include <exe/coro/sync/wait_group.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>

#include <twist/assist/shared.hpp>
#include <twist/test/assert.hpp>

#include <array>

using namespace exe;  // NOLINT

TEST_SUITE(RandomizeWaitGroup) {
  TWIST_RANDOMIZE(OneWaiter, 5s) {
    sched::ThreadPool scheduler{3};
    scheduler.Start();

    {
      thread::WaitGroup test;

      const size_t kCoroutines = 4;

      test.Add(1);

      auto main = [&] -> coro::Go {
        co_await coro::JumpTo(scheduler);

        coro::WaitGroup wg;

        std::array<twist::assist::Shared<bool>, kCoroutines> flags = {false};
        twist::ed::std::atomic_size_t left{kCoroutines};

        auto worker = [&](size_t i) -> coro::Go {
          co_await coro::JumpTo(scheduler);

          for (size_t j = 0; j < 3; ++j) {
            co_await coro::Yield();
          }

          flags[i].Write(true);
          left.fetch_sub(1);
          wg.Done();
        };

        for (size_t i = 0; i < kCoroutines; ++i) {
          wg.Add(1);
          worker(i);
        }

        co_await wg.Wait();

        TWIST_TEST_ASSERT(left.load() == 0, "Unfinished coroutines");

        for (size_t i = 0; i < kCoroutines; ++i) {
          bool f = flags[i].Read();
          TWIST_TEST_ASSERT(f, "Missing work");
        }

        test.Done();
      };

      main();

      test.Wait();
    }

    scheduler.Stop();
  }

  TWIST_RANDOMIZE(Waiters, 10s) {
    sched::ThreadPool scheduler{3};
    scheduler.Start();

    {
      thread::WaitGroup test;

      const size_t kWorkers = 3;
      const size_t kWaiters = 3;

      test.Add(1);

      auto main = [&] -> coro::Go {
        co_await coro::JumpTo(scheduler);

        std::array<twist::assist::Shared<bool>, kWorkers> flags = {false};

        coro::WaitGroup work;
        coro::WaitGroup wait;

        auto worker = [&](size_t i) -> coro::Go {
          co_await coro::JumpTo(scheduler);

          for (size_t k = 0; k < 3; ++k) {
            co_await coro::Yield();
          }

          flags[i].Write(true);
          work.Done();
        };

        for (size_t i = 0; i < kWorkers; ++i) {
          work.Add(1);
          worker(i);
        }

        auto waiter = [&] -> coro::Go {
          co_await coro::JumpTo(scheduler);

          for (size_t k = 0; k < 3; ++k) {
            co_await coro::Yield();
          }

          co_await work.Wait();

          for (size_t i = 0; i < kWorkers; ++i) {
            bool f = flags[i].Read();
            TWIST_TEST_ASSERT(f, "Missing work");
          }

          wait.Done();
        };

        for (size_t j = 0; j < kWaiters; ++j) {
          wait.Add(1);
          waiter();
        }

        co_await wait.Wait();

        test.Done();
      };

      main();

      test.Wait();
    }

    scheduler.Stop();
  }

  TWIST_RANDOMIZE(Cyclic, 10s) {
    sched::ThreadPool scheduler{3};
    scheduler.Start();

    const size_t kIters = 3;

    {
      thread::WaitGroup test;

      const size_t kWorkers = 3;

      test.Add(1);

      auto main = [&] -> coro::Go {
        co_await coro::JumpTo(scheduler);

        std::array<twist::assist::Shared<bool>, kWorkers> flags = {false};
        coro::WaitGroup work;

        auto worker = [&](size_t i) -> coro::Go {
          co_await coro::JumpTo(scheduler);

          flags[i].Write(true);
          work.Done();
        };

        for (size_t k = 0; k < kIters; ++k) {
          // New epoch

          for (size_t i = 0; i < kWorkers; ++i) {
            work.Add(1);
            worker(i);
          }

          co_await work.Wait();

          for (size_t i = 0; i < kWorkers; ++i) {
            bool f = flags[i].Read();
            TWIST_TEST_ASSERT(f, "Missing work");
          }
        }

        test.Done();
      };

      main();

      test.Wait();
    }

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
