#include <exe/sched/thread_pool.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sync/event.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>

#include <twist/assist/shared.hpp>
#include <twist/test/assert.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(RandomizeEvent) {
  TWIST_RANDOMIZE(MessagePassing, 10s) {
    sched::ThreadPool scheduler{3};
    scheduler.Start();

    {
      thread::WaitGroup test;

      twist::assist::Shared<int> data = 0;
      coro::Event event;

      auto producer = [&] -> coro::Go {
        co_await coro::JumpTo(scheduler);

        data.Write(1);
        event.Fire();

        test.Done();
      };

      auto consumer = [&] -> coro::Go {
        co_await coro::JumpTo(scheduler);

        co_await event.Wait();

        int d = data.Read();
        TWIST_TEST_ASSERT(d == 1, "Unfinished work");

        test.Done();
      };

      test.Add(2);
      consumer();
      producer();

      test.Wait();
    }

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
