#include <exe/sched/thread_pool.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sync/mutex_flavor/run.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>

#include <twist/assist/random.hpp>

#include <twist/test/assert.hpp>
#include <twist/test/plate.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(RandomizeRunMutex) {
  TWIST_RANDOMIZE(Lock, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t coros = choice(1, 6);
    size_t writes = choice(1, 5);


    sched::ThreadPool scheduler{3};
    scheduler.Start();

    {
      thread::WaitGroup test;

      coro::mutex_flavor::RunMutex mutex;
      twist::test::Plate plate;

      auto locker = [&] -> coro::Go {
        co_await coro::JumpTo(scheduler);

        for (size_t j = 0; j < writes; ++j) {
          co_await mutex.Run([&plate] {
            plate.Access();
          });
        }

        test.Done();
      };

      for (size_t i = 0; i < coros; ++i) {
        test.Add(1);
        locker();
      }

      test.Wait();

      TWIST_TEST_ASSERT(plate.AccessCount() == coros * writes, "Missing cs");
    }

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
