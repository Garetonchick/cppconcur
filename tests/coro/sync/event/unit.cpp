#include <exe/sched/run_loop.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sync/event.hpp>

#include <wheels/test/framework.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(Event) {
  SIMPLE_TEST(WaitReady) {
    sched::RunLoop loop;

    const std::string kMessage = "Hello";

    coro::Event event;
    std::string data;
    bool ok = false;

    auto consumer = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      co_await event.Wait();
      ASSERT_EQ(data, kMessage);
      ok = true;
    };

    auto producer = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      data = kMessage;
      event.Fire();
    };

    producer();
    consumer();

    loop.Run();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(WaitSuspend) {
    sched::RunLoop loop;

    coro::Event event;
    bool ok = false;

    auto consumer = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      co_await event.Wait();
      ok = true;
    };

    consumer();

    {
      size_t tasks = loop.Run();
      ASSERT_LE(tasks, 7);
    }

    ASSERT_FALSE(ok);

    auto producer = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      event.Fire();
    };

    producer();

    loop.Run();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(Waiters) {
    sched::RunLoop loop;

    coro::Event event;
    bool work = false;
    size_t wait = 0;

    const size_t kConsumers = 7;

    auto consumer = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      co_await event.Wait();
      ASSERT_TRUE(work);
      ++wait;
    };

    for (size_t i = 0; i < kConsumers; ++i) {
      consumer();
    }

    auto producer = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      work = true;
      event.Fire();
    };

    producer();

    loop.Run();

    ASSERT_EQ(wait, kConsumers);
  }
}

RUN_ALL_TESTS()
