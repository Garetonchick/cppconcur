#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/strand.hpp>

#include <exe/thread/wait_group.hpp>

#include <twist/assist/random.hpp>

#include <twist/test/plate.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(StressStrand) {
  TWIST_STRESS_TEST(Iter, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    sched::ThreadPool scheduler{3};
    scheduler.Start();

    course::test::TimeBudget time_budget;

    size_t iter_count = 0;

    while (time_budget) {
      ++iter_count;

      // Parameters
      size_t fibers = choice(1, 6);
      size_t writes = choice(1, 5);

      thread::WaitGroup test_iter;

      twist::test::Plate plate;
      fiber::Strand mutex;

      for (size_t i = 0; i < fibers; ++i) {
        test_iter.Add(1);

        fiber::Go(scheduler, [&] {
          for (size_t j = 0; j < writes; ++j) {
            mutex.Combine([&] {
              plate.Access();
            });
          }

          test_iter.Done();
        });
      }

      test_iter.Wait();
    }

    scheduler.Stop();

    fmt::println("# iterations = {}", iter_count);
  }

  TWIST_STRESS_TEST(Load1, 5s) {
    // Parameters
    const size_t kFibers = 11;
    const size_t kThreads = 3;

    sched::ThreadPool scheduler{kThreads};
    scheduler.Start();

    thread::WaitGroup wg;

    fiber::Strand mutex;
    twist::test::Plate plate;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);

      fiber::Go(scheduler, [&] {
        course::test::TimeBudget time_budget;

        while (time_budget) {
          mutex.Combine([&] {
            plate.Access();
          });
        }

        wg.Done();
      });
    }

    wg.Wait();

    fmt::println("#cs = {}", plate.AccessCount());

    scheduler.Stop();
  }

  TWIST_STRESS_TEST(Load2, 5s) {
    // Parameters
    const size_t kFibers = 48;
    const size_t kThreads = 4;

    sched::ThreadPool scheduler{kThreads};
    scheduler.Start();

    thread::WaitGroup wg;

    fiber::Strand mutex;
    twist::test::Plate plate;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);

      fiber::Go(scheduler, [&] {
        course::test::TimeBudget time_budget;

        while (time_budget) {
          mutex.Combine([&] {
            plate.Access();
          });
        }

        wg.Done();
      });
    }

    wg.Wait();

    fmt::println("#cs = {}", plate.AccessCount());

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
