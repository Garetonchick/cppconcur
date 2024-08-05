#include <exe/sched/thread_pool.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sync/mutex_flavor/run.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/assist/random.hpp>

#include <twist/test/plate.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(StressRunMutex) {
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
      size_t coros = choice(1, 6);
      size_t writes = choice(1, 5);

      thread::WaitGroup test_iter;

      twist::test::Plate plate;
      coro::mutex_flavor::RunMutex mutex;

      auto locker = [&] -> coro::Go {
        co_await coro::JumpTo(scheduler);

        for (size_t j = 0; j < writes; ++j) {
          co_await mutex.Run([&plate] {
            plate.Access();
          });
        }

        test_iter.Done();
      };

      for (size_t i = 0; i < coros; ++i) {
        test_iter.Add(1);
        locker();
      }

      test_iter.Wait();
    }

    scheduler.Stop();

    fmt::println("# iterations = {}", iter_count);
  }

  TWIST_STRESS_TEST(Load, 5s) {
    // Parameters
    const size_t kCoroutines = 11;
    const size_t kThreads = 3;

    sched::ThreadPool scheduler{kThreads};
    scheduler.Start();

    thread::WaitGroup wg;

    coro::mutex_flavor::RunMutex mutex;
    twist::test::Plate plate;

    auto locker = [&] -> coro::Go {
      co_await coro::JumpTo(scheduler);

      course::test::TimeBudget time_budget;

      while (time_budget) {
        co_await mutex.Run([&plate] {
          plate.Access();
        });
      }

      wg.Done();
    };

    for (size_t i = 0; i < kCoroutines; ++i) {
      wg.Add(1);
      locker();
    }

    wg.Wait();

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
