#include <course/test/twist.hpp>

#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>
#include <exe/fiber/sync/wait_group.hpp>

#include <exe/thread/wait_group.hpp>

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

      const size_t kFibers = 4;

      std::array<twist::assist::Shared<bool>, kFibers> flags = {false};

      test.Add(1);

      fiber::Go(scheduler, [&] {
        fiber::WaitGroup wg;

        twist::ed::std::atomic_size_t left{kFibers};

        for (size_t i = 0; i < kFibers; ++i) {
          wg.Add(1);

          fiber::Go([&, i] {
            for (size_t j = 0; j < 3; ++j) {
              fiber::Yield();
            }

            flags[i].Write(true);
            left.fetch_sub(1);
            wg.Done();
          });
        }

        wg.Wait();

        TWIST_TEST_ASSERT(left.load() == 0, "Unfinished fibers");

        for (size_t i = 0; i < kFibers; ++i) {
          bool f = flags[i].Read();
          TWIST_TEST_ASSERT(f, "Missing work");
        }

        test.Done();
      });

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

      std::array<twist::assist::Shared<bool>, kWorkers> flags = {false};

      test.Add(1);

      fiber::Go(scheduler, [&] {
        fiber::WaitGroup work;
        fiber::WaitGroup wait;

        for (size_t i = 0; i < kWorkers; ++i) {
          work.Add(1);

          fiber::Go([&, i] {
            for (size_t k = 0; k < 3; ++k) {
              fiber::Yield();
            }

            flags[i].Write(true);
            work.Done();
          });
        }

        for (size_t j = 0; j < kWaiters; ++j) {
          wait.Add(1);

          fiber::Go([&] {
            for (size_t k = 0; k < 3; ++k) {
              fiber::Yield();
            }

            work.Wait();

            for (size_t i = 0; i < kWorkers; ++i) {
              bool f = flags[i].Read();
              TWIST_TEST_ASSERT(f, "Missing work");
            }

            wait.Done();
          });
        }

        wait.Wait();
        test.Done();
      });

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

      std::array<twist::assist::Shared<bool>, kWorkers> flags = {false};

      test.Add(1);

      fiber::Go(scheduler, [&] {
        fiber::WaitGroup work;

        for (size_t k = 0; k < kIters; ++k) {
          // New epoch

          for (size_t i = 0; i < kWorkers; ++i) {
            work.Add(1);

            fiber::Go([&, i] {
              flags[i].Write(true);
              work.Done();
            });
          }

          work.Wait();

          for (size_t i = 0; i < kWorkers; ++i) {
            bool f = flags[i].Read();
            TWIST_TEST_ASSERT(f, "Missing work");
          }
        }

        test.Done();
      });

      test.Wait();
    }

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
