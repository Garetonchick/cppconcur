#include <course/test/twist.hpp>

#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/mutex.hpp>

#include <exe/thread/wait_group.hpp>

#include <twist/assist/random.hpp>

#include <twist/test/plate.hpp>
#include <twist/test/either.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(RandomizeMutex) {
  TWIST_RANDOMIZE(Lock, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t fibers = choice(1, 6);
    size_t writes = choice(1, 5);


    sched::ThreadPool scheduler{3};
    scheduler.Start();

    {
      thread::WaitGroup test;

      fiber::Mutex mutex;
      twist::test::Plate plate;

      for (size_t i = 0; i < fibers; ++i) {
        test.Add(1);

        fiber::Go(scheduler, [&] {
          for (size_t j = 0; j < writes; ++j) {
            mutex.Lock();
            plate.Access();
            mutex.Unlock();
          }

          test.Done();
        });
      }

      test.Wait();
    }

    scheduler.Stop();
  }

  TWIST_RANDOMIZE(LockAndTryLock, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t fibers = choice(1, 6);
    size_t writes = choice(1, 5);


    sched::ThreadPool scheduler{3};
    scheduler.Start();

    {
      thread::WaitGroup test;

      fiber::Mutex mutex;
      twist::test::Plate plate;

      for (size_t i = 0; i < fibers; ++i) {
        test.Add(1);

        fiber::Go(scheduler, [&] {
          for (size_t j = 0; j < writes; ++j) {
            if (twist::test::Either()) {
              mutex.Lock();
              plate.Access();
              mutex.Unlock();
            } else {
              if (mutex.TryLock()) {
                plate.Access();
                mutex.Unlock();
              }
            }
          }

          test.Done();
        });
      }

      test.Wait();
    }

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
