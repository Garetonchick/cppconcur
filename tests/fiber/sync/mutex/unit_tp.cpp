#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/mutex.hpp>

#include <exe/thread/wait_group.hpp>

#include <wheels/test/framework.hpp>

#include <thread>

using namespace exe;  // NOLINT

using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(Mutex) {
  TEST(UnlockFork, wheels::test::TestOptions().TimeLimit(10s)) {
    sched::ThreadPool scheduler{4};
    scheduler.Start();

    fiber::Mutex mutex;

    thread::WaitGroup test;
    test.Add(4);

    fiber::Go(scheduler, [&] {
      mutex.Lock();

      std::this_thread::sleep_for(1s);

      mutex.Unlock();

      test.Done();
    });

    for (size_t i = 0; i < 3; ++i) {
      fiber::Go(scheduler, [&] {
        mutex.Lock();
        // Sequential
        mutex.Unlock();

        // Parallel
        std::this_thread::sleep_for(5s);

        test.Done();
      });
    }

    test.Wait();

    scheduler.Stop();
  }
}

RUN_ALL_TESTS()
