#include <exe/sched/run_loop.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>

#include <twist/sim.hpp>

#include <twist/test/assert.hpp>

#include <wheels/test/framework.hpp>

static_assert(twist::build::IsolatedSim());

using namespace exe;  // NOLINT

TEST_SUITE(Yield) {
  SIMPLE_TEST(NoSynchronization) {
    twist::sim::sched::RandomScheduler scheduler;
    twist::sim::Simulator simulator{&scheduler};

    auto result = simulator.Run([] {
      sched::RunLoop loop;

      for (size_t i = 0; i < 11; ++i) {
        fiber::Go(loop, [] {
          for (size_t i = 0; i < 27; ++i) {
            fiber::Yield();
          }
        });
      }

      loop.Run();

      TWIST_TEST_ASSERT(twist::sim::stat::AtomicCount() <= 3, "Synchronization detected");
    });

    ASSERT_TRUE_M(result.Ok(), result.std_err);
  }
}

RUN_ALL_TESTS()
