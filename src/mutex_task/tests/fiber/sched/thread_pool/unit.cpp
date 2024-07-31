#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>

#include <exe/thread/wait_group.hpp>

#include <wheels/core/stop_watch.hpp>

#include <wheels/test/framework.hpp>

#include <thread>
#include <chrono>

using namespace std::chrono_literals;  // NOLINT

using namespace exe;  // NOLINT

void ExpectScheduler(sched::ThreadPool& pool) {
  ASSERT_EQ(sched::ThreadPool::Current(), &pool);
}

TEST_SUITE(Fibers) {
  SIMPLE_TEST(Go) {
    sched::ThreadPool pool{3};
    pool.Start();

    thread::WaitGroup wg;

    wg.Add(1);
    fiber::Go(pool, [&]() {
      ExpectScheduler(pool);
      wg.Done();
    });

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(GoGroup) {
    sched::ThreadPool pool{4};
    pool.Start();

    thread::WaitGroup wg;

    const size_t kFibers = 7;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);
      fiber::Go(pool, [&] {
        ExpectScheduler(pool);
        wg.Done();
      });
    }

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(GoChild) {
    sched::ThreadPool pool{3};
    pool.Start();

    thread::WaitGroup wg;

    wg.Add(1);

    fiber::Go(pool, [&] {
      ExpectScheduler(pool);

      wg.Add(1);
      fiber::Go([&] {
        ExpectScheduler(pool);
        wg.Done();
      });

      wg.Done();
    });

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(Parallel) {
    const size_t kThreads = 4;

    sched::ThreadPool pool{kThreads};
    pool.Start();

    wheels::StopWatch stop_watch;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kThreads; ++i) {
      wg.Add(1);
      fiber::Go(pool, [&wg] {
        std::this_thread::sleep_for(1s);
        wg.Done();
      });
    }

    wg.Wait();

    ASSERT_TRUE(stop_watch.Elapsed() < 2s);

    pool.Stop();
  }

  SIMPLE_TEST(Yield) {
    sched::ThreadPool pool{1};
    pool.Start();

    thread::WaitGroup wg;

    wg.Add(1);
    fiber::Go(pool, [&] {
      fiber::Yield();

      ExpectScheduler(pool);

      wg.Done();
    });

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(YieldChild) {
    sched::ThreadPool loop{1};
    loop.Start();

    thread::WaitGroup wg;

    wg.Add(1);
    fiber::Go(loop, [&wg] {
      bool child = false;

      fiber::Go([&] {
        child = true;
      });

      while (!child) {
        fiber::Yield();
      }

      wg.Done();
    });

    wg.Wait();

    loop.Stop();
  }

  SIMPLE_TEST(ForYield) {
    sched::ThreadPool loop{1};
    loop.Start();

    const size_t kYields = 128;

    size_t yields = 0;

    thread::WaitGroup wg;

    wg.Add(1);

    fiber::Go(loop, [&] {
      for (size_t i = 0; i < kYields; ++i) {
        fiber::Yield();
        ++yields;
      }

      wg.Done();
    });

    wg.Wait();

    ASSERT_EQ(yields, kYields);

    loop.Stop();
  }

  SIMPLE_TEST(PingPong) {
    sched::ThreadPool loop{1};
    loop.Start();

    bool start = false;
    int turn = 0;

    thread::WaitGroup wg;

    wg.Add(2);

    const size_t kRounds = 3;

    fiber::Go(loop, [&] {
      while (!start) {
        fiber::Yield();
      }

      for (size_t i = 0; i < kRounds; ++i) {
        ASSERT_TRUE(turn == 0)
        turn ^= 1;

        fiber::Yield();
      }

      wg.Done();
    });

    fiber::Go(loop, [&] {
      {
        start = true;
        fiber::Yield();
      }

      for (size_t j = 0; j < kRounds; ++j) {
        ASSERT_TRUE(turn == 1);
        turn ^= 1;

        fiber::Yield();
      }

      wg.Done();
    });

    wg.Wait();

    loop.Stop();
  }

  SIMPLE_TEST(YieldGroup) {
    sched::ThreadPool pool{4};
    pool.Start();

    const size_t kFibers = 5;
    const size_t kYields = 7;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);

      fiber::Go(pool, [&] {
        for (size_t j = 0; j < kYields; ++j) {
          fiber::Yield();
        }
        wg.Done();
      });
    }

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(TwoPools) {
    sched::ThreadPool pool1{3};
    pool1.Start();

    sched::ThreadPool pool2{3};
    pool2.Start();

    thread::WaitGroup wg;

    wg.Add(2);

    fiber::Go(pool1, [&] {
      for (size_t i = 0; i < 2; ++i) {
        fiber::Yield();
        ExpectScheduler(pool1);
      }
      wg.Done();
    });

    fiber::Go(pool2, [&] {
      for (size_t j = 0; j < 3; ++j) {
        fiber::Yield();
        ExpectScheduler(pool2);
      }
      wg.Done();
    });

    wg.Wait();

    pool1.Stop();
    pool2.Stop();
  }
}

RUN_ALL_TESTS()
