#include <course/test/twist.hpp>

#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/event.hpp>

#include <exe/thread/wait_group.hpp>

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
      fiber::Event event;

      test.Add(1);

      fiber::Go(scheduler, [&] {
        data.Write(1);
        event.Fire();

        test.Done();
      });

      test.Add(1);

      fiber::Go(scheduler, [&] {
        event.Wait();

        int d = data.Read();
        TWIST_TEST_ASSERT(d == 1, "Unfinished work");

        test.Done();
      });

      test.Wait();
    }

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
