#include <exe/sched/run_loop.hpp>

#include <twist/sim.hpp>

#include <wheels/test/framework.hpp>

#include <fmt/core.h>

#include <array>

static_assert(twist::build::IsolatedSim());

using namespace exe;  // NOLINT

TEST_SUITE(RunLoop) {
  struct Job final : sched::task::TaskBase {
    void Run() noexcept override {
      // No-op
    }
  };

  SIMPLE_TEST(NoAllocations1) {
    twist::sim::sched::CoopScheduler scheduler;
    twist::sim::Simulator simulator{&scheduler};

    auto result = simulator.Run([] {
      sched::RunLoop loop;

      size_t alloc_count_1 = twist::sim::stat::AllocationCount();

      Job job;
      loop.Submit(&job);
      size_t done = loop.Run();
      ASSERT_TRUE(done == 1);

      size_t alloc_count_2 = twist::sim::stat::AllocationCount();

      ASSERT_EQ(alloc_count_1, alloc_count_2);
    });

    ASSERT_TRUE(result.Ok());
  }

  SIMPLE_TEST(NoAllocations2) {
    twist::sim::sched::CoopScheduler scheduler;
    twist::sim::Simulator simulator{&scheduler};

    auto result = simulator.Run([] {
      sched::RunLoop loop;

      size_t alloc_count_1 = twist::sim::stat::AllocationCount();

      std::array<Job, 256> jobs = {};

      for (size_t i = 0; i < jobs.size(); ++i) {
        loop.Submit(&jobs[i]);
      }

      size_t done = loop.Run();
      ASSERT_TRUE(done == jobs.size());

      size_t alloc_count_2 = twist::sim::stat::AllocationCount();

      ASSERT_EQ(alloc_count_1, alloc_count_2);
    });

    ASSERT_TRUE_M(result.Ok(), result.std_err);
  }
}

RUN_ALL_TESTS()
