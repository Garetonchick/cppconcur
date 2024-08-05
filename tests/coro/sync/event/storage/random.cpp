#include <exe/sched/thread_pool.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sync/event.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(RandomizeEvent) {
  TWIST_RANDOMIZE(Storage, 5s) {
    sched::ThreadPool scheduler{3};
    scheduler.Start();

    {
      thread::WaitGroup test;

      auto test_case = [&] -> coro::Go {
        co_await coro::JumpTo(scheduler);

        auto* event = new coro::Event{};

        auto producer = [&scheduler, event] -> coro::Go {
          co_await coro::JumpTo(scheduler);

          event->Fire();
        };

        producer();

        co_await event->Wait();
        delete event;

        test.Done();
      };

      test.Add(1);
      test_case();
      test.Wait();
    }

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
