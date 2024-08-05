#include <exe/sched/thread_pool.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/current.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sched/yield.hpp>

#include <exe/thread/wait_group.hpp>

#include <wheels/test/framework.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(Coroutines) {
  SIMPLE_TEST(JumpTo) {
    sched::ThreadPool pool{3};
    pool.Start();

    thread::WaitGroup wg;

    auto co = [&] -> coro::Go {
      co_await coro::JumpTo(pool);
      wg.Done();
    };

    wg.Add(1);
    co();

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(JumpToGroup) {
    sched::ThreadPool pool{3};
    pool.Start();

    thread::WaitGroup wg;

    auto co = [&] -> coro::Go {
      co_await coro::JumpTo(pool);
      wg.Done();
    };

    for (size_t i = 0; i < 10; ++i) {
      wg.Add(1);
      co();
    }

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(Yield) {
    sched::ThreadPool pool{3};
    pool.Start();

    thread::WaitGroup wg;

    auto co = [&] -> coro::Go {
      co_await coro::JumpTo(pool);

      for (size_t i = 0; i < 7; ++i) {
        co_await coro::Yield();
      }

      wg.Done();
    };

    wg.Add(1);
    co();

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(CurrentScheduler) {
    sched::ThreadPool pool{4};
    pool.Start();

    thread::WaitGroup wg;

    auto co = [&] -> coro::Go {
      co_await coro::JumpTo(pool);

      {
        auto* scheduler = co_await coro::CurrentScheduler();
        ASSERT_EQ(scheduler, &pool);
      }

      co_await coro::Yield();

      {
        auto* scheduler = co_await coro::CurrentScheduler();
        ASSERT_EQ(scheduler, &pool);
      }

      wg.Done();
    };

    wg.Add(1);
    co();

    wg.Wait();

    pool.Stop();
  }
}

RUN_ALL_TESTS()
