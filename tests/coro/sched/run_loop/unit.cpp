#include <exe/sched/run_loop.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/current.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sched/yield.hpp>

#include <wheels/test/framework.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(Coroutines) {
  SIMPLE_TEST(FireAndForget) {
    bool done = false;

    auto co = [&] -> coro::Go {
      done = true;
      co_return;
    };

    co();

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(JumpTo) {
    sched::RunLoop loop;

    bool done = false;

    auto co = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      done = true;
    };

    co();

    ASSERT_FALSE(done);

    size_t tasks = loop.Run();
    ASSERT_EQ(tasks, 1);

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(JumpToGroup) {
    sched::RunLoop loop;

    const size_t kCoroutines = 7;

    auto co = [&loop] -> coro::Go {
      co_await coro::JumpTo(loop);
    };

    for (size_t i = 0; i < kCoroutines; ++i) {
      co();
    }

    size_t tasks = loop.Run();
    ASSERT_EQ(tasks, kCoroutines);
  }

  SIMPLE_TEST(FireChild) {
    sched::RunLoop loop;

    bool flag = false;

    auto child = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      flag = true;
    };

    auto parent = [&] -> coro::Go {
      co_await coro::JumpTo(loop);
      child();
    };

    parent();

    ASSERT_TRUE(loop.RunNext());
    ASSERT_FALSE(flag);
    ASSERT_EQ(loop.Run(), 1);
    ASSERT_TRUE(flag);
  }

  SIMPLE_TEST(Yield) {
    sched::RunLoop loop;

    auto co = [&loop] -> coro::Go {
      co_await coro::JumpTo(loop);

      co_await coro::Yield();
    };

    co();

    ASSERT_EQ(loop.Run(), 2);
  }

  SIMPLE_TEST(PingPong) {
    sched::RunLoop loop;

    int turn = 0;

    auto ping = [&] -> coro::Go{
      co_await coro::JumpTo(loop);

      for (size_t i = 0; i < 3; ++i) {
        ASSERT_EQ(turn, 0);
        turn ^= 1;
        co_await coro::Yield();
      }
    };

    auto pong = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      for (size_t j = 0; j < 3; ++j) {
        ASSERT_EQ(turn, 1);
        turn ^= 1;
        co_await coro::Yield();
      }
    };

    ping();
    pong();

    loop.Run();
  }

  SIMPLE_TEST(YieldGroup) {
    sched::RunLoop loop;

    const size_t kCoroutines = 3;
    const size_t kYields = 4;

    auto co = [&loop] -> coro::Go {
      co_await coro::JumpTo(loop);

      for (size_t k = 0; k < kYields; ++k) {
        co_await coro::Yield();
      }
    };

    for (size_t i = 0; i < kCoroutines; ++i) {
      co();
    }

    size_t tasks = loop.Run();
    ASSERT_EQ(tasks, kCoroutines * (kYields + 1));
  }

  SIMPLE_TEST(CurrentScheduler) {
    sched::RunLoop loop;

    bool done = false;

    auto co = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      {
        auto* scheduler = co_await coro::CurrentScheduler();
        ASSERT_EQ(scheduler, &loop);
      }

      co_await coro::Yield();

      {
        auto* scheduler = co_await coro::CurrentScheduler();
        ASSERT_EQ(scheduler, &loop);
      }

      done = true;
    };

    co();

    ASSERT_FALSE(done);

    size_t tasks = loop.Run();
    ASSERT_EQ(tasks, 2);

    ASSERT_TRUE(done);
  }
}

RUN_ALL_TESTS()
