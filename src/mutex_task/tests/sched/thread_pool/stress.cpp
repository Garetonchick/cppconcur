#include <exe/sched/thread_pool.hpp>
#include <exe/sched/task/submit.hpp>
#include <exe/thread/wait_group.hpp>

#include <twist/ed/std/atomic.hpp>

#include <twist/test/assert.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(StressThreadPool) {
  TWIST_STRESS_TEST(SubmitAndWait, 5s) {
    sched::ThreadPool pool{4};
    pool.Start();

    course::test::TimeBudget time_budget;

    for (size_t iter = 0; time_budget; ++iter) {
      size_t todo = 1 + iter % 11;
      twist::ed::std::atomic_size_t done{0};

      thread::WaitGroup wg;
      for (size_t i = 0; i < todo; ++i) {
        wg.Add(1);
        sched::task::Submit(pool, [&] {
          ++done;
          wg.Done();
        });
      }

      wg.Wait();

      TWIST_TEST_ASSERT(done.load() == todo, "Missing tasks");
    }

    pool.Stop();
  }
}

RUN_ALL_TESTS()
