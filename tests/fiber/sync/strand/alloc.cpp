#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/strand.hpp>

#include <exe/thread/wait_group.hpp>

#include <twist/ed/std/thread.hpp>

#include <wheels/test/framework.hpp>
#include <course/test/twist.hpp>

#include <twist/mod/sim/stat/memory.hpp>

#include <twist/test/assert.hpp>
#include <twist/test/plate.hpp>

#include <fmt/core.h>

static_assert(twist::build::IsolatedSim());

using namespace exe;  // NOLINT

TEST_SUITE(RandomizeStrand) {
  struct Luggage {
    twist::test::Plate& plate;
    char space[128] = {0};

    explicit Luggage(twist::test::Plate& p)
        : plate(p) {
    }
  };

  TWIST_RANDOMIZE(NoAllocations, 10s) {
    sched::ThreadPool pool{3};
    pool.Start();

    thread::WaitGroup wg;

    constexpr size_t kFibers = 5;
    constexpr size_t kIters = 11;

    twist::test::Plate plate;
    fiber::Strand mutex;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);

      fiber::Go(pool, [&] {
        twist::ed::std::this_thread::yield();

        for (size_t j = 0; j < kIters; ++j) {
          mutex.Combine([luggage = Luggage{plate}] {
            luggage.plate.Access();
          });
        }

        wg.Done();
      });
    }

    size_t alloc_count1 = twist::sim::stat::AllocationCount();

    wg.Wait();

    TWIST_TEST_ASSERT(plate.AccessCount() == kFibers * kIters, "Missing critical sections");

    size_t alloc_count2 = twist::sim::stat::AllocationCount();

    TWIST_TEST_ASSERT(alloc_count1 == alloc_count2, "Dynamic memory allocations");

    pool.Stop();
  }
}

RUN_ALL_TESTS()
