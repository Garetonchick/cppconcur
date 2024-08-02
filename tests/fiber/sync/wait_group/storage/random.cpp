#include <course/test/twist.hpp>

#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/wait_group.hpp>

#include <exe/thread/wait_group.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(WaitGroup) {
  TWIST_RANDOMIZE(Storage, 5s) {
    sched::ThreadPool scheduler{3};
    scheduler.Start();

    {
      thread::WaitGroup test;

      test.Add(1);

      fiber::Go(scheduler, [&test] {
        auto* wg = new fiber::WaitGroup{};

        wg->Add(1);
        fiber::Go([wg] {
          wg->Done();
        });

        wg->Wait();
        delete wg;

        test.Done();
      });

      test.Wait();
    }

    scheduler.Stop();
  }
}

RUN_ALL_TESTS();
