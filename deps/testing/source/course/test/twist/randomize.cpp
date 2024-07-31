#include "randomize.hpp"

#include <course/test/time_budget.hpp>

#include <wheels/test/framework.hpp>

#include <fmt/core.h>

#if defined(__TWIST_SIM__)

// simulation

#include <twist/sim.hpp>

#if defined(__TWIST_SIM_ISOLATION__)

// matrix

#include "sim/result.hpp"

namespace course::test::twist {
namespace randomize {

std::chrono::milliseconds TestTimeLimit(std::chrono::milliseconds budget) {
  // Reserve time for execution printing
  return budget + std::chrono::seconds(30);
}

using ::twist::sim::sched::RandomScheduler;
using ::twist::sim::sched::PctScheduler;
using ::twist::sim::Simulator;

::twist::sim::Result RunWithLimit(Simulator& simulator, TestBody body, size_t limit = 4096) {
  simulator.Start(body);
  size_t iters = simulator.RunFor(limit);

  if (iters >= limit) {
    simulator.Burn();

    // Print

    Simulator replay_simulator{simulator.Scheduler()};

    static const size_t kReplayIterLimit = 64;

    replay_simulator.Start(body);
    replay_simulator.Silent(true);
    ::twist::sim::StdoutPrinter printer;
    replay_simulator.SetLogger(&printer);
    replay_simulator.AllowSysLogging(true);
    replay_simulator.RunFor(kReplayIterLimit);

    fmt::println("... (output truncated after {} scheduling iterations)", kReplayIterLimit);

    replay_simulator.Burn();

    std::cout.flush();

    FAIL_TEST(fmt::format("Execution took too long (> {} scheduling iterations) and was interrupted", limit));

    std::abort();

  } else {
    return simulator.Drain();  // ~ Stop runloop
  }
}

size_t EstimateSize(TestBody body) {
  ::twist::sim::sched::RandomScheduler scheduler;
  ::twist::sim::Simulator simulator{&scheduler};
  auto result = RunWithLimit(simulator, body);
  return result.iters;
};

struct Replay {
  ::twist::sim::sched::Schedule schedule;
  ::twist::sim::Digest digest;
};

Replay TryRecord(TestBody body, RandomScheduler& scheduler) {
  ::twist::sim::sched::Recorder recorder{&scheduler};
  ::twist::sim::Simulator simulator{&recorder};
  auto result = simulator.Run(body);
  return {recorder.GetSchedule(), result.digest};
}

void Check(TestBody body, Params params) {
  if (!::twist::sim::DetCheck(body)) {
    FAIL_TEST("Test routine is not deterministic");
  }

  {
    static const size_t kTestSizeLimit = 10'000;

    if (EstimateSize(body) > kTestSizeLimit) {
      FAIL_TEST("Test is too large for randomized testing: >" << kTestSizeLimit << " steps");
    }
  }

  course::test::TimeBudget time_budget{params.time_budget};

  auto sched_params = RandomScheduler::Params{};
  sched_params.time_slice = 13;
  RandomScheduler scheduler{sched_params};

  size_t sim_count = 0;

  do {
    Simulator simulator{&scheduler};

    auto result = RunWithLimit(simulator, body);

    ++sim_count;

    if (result.Failure()) {
      static const size_t kNotTooLarge = 256;

      if (result.iters < kNotTooLarge) {
        auto replay = TryRecord(body, scheduler);
        if (replay.digest != result.digest) {
          WHEELS_PANIC("Twist error: failed to record simulation replay");
        }

        ::twist::sim::Print(body, replay.schedule);
        std::cout.flush();
      }

      FAIL_TEST(sim::FormatFailure(result));
    }
  } while (scheduler.NextSchedule() && time_budget);

  fmt::println("Executions checked: {}", sim_count);
}

}  // namespace randomize
}  // namespace course::test::twist

#else

// Simulation, maybe with sanitizers

namespace course::test::twist {
namespace randomize {

std::chrono::milliseconds TestTimeLimit(std::chrono::milliseconds budget) {
  return budget + std::chrono::seconds(1);
}

using ::twist::sim::sched::RandomScheduler;
using ::twist::sim::sched::PctScheduler;

size_t EstimateSize(TestBody body) {
  ::twist::sim::sched::RandomScheduler scheduler;
  ::twist::sim::Simulator simulator{&scheduler};
  auto result = simulator.Run(body);
  return result.iters;
};

void Check(TestBody body, Params params) {
  // TODO: PctScheduler

  if (EstimateSize(body) > 1024) {
    FAIL_TEST("Test is too large for random checking");
  }

  auto sched_params = RandomScheduler::Params{};
  sched_params.time_slice = 13;
  RandomScheduler scheduler{sched_params};

  size_t sim_count = 0;

  course::test::TimeBudget time_budget{params.time_budget};

  do {
    ::twist::sim::Simulator simulator{&scheduler};
    auto result = simulator.Run(body);

    ++sim_count;

    if (result.Failure()) {
      // Impossible?
    }

  } while (scheduler.NextSchedule() && time_budget);

  fmt::println("Executions checked: {}", sim_count);
}

}  // namespace randomize
}  // namespace course::test::twist

#endif

#elif defined(__TWIST_FAULTY__)

// threads + fault injection

#include <twist/rt/thr/fault/adversary/adversary.hpp>

#include <twist/rt/thr/logging/logging.hpp>

#include <twist/test/new_iter.hpp>

namespace course::test::twist {
namespace randomize {

std::chrono::milliseconds TestTimeLimit(std::chrono::milliseconds budget) {
  return budget + std::chrono::seconds(1);
}

void TestIter(size_t iter, TestBody body, Params) {
  body();

  ::twist::rt::thr::log::FlushPendingLogMessages();

  ::twist::rt::thr::fault::Adversary()->Iter(iter);
}

void Check(TestBody body, Params params) {
  ::twist::rt::thr::fault::Adversary()->Reset();

  course::test::TimeBudget time_budget{params.time_budget};

  size_t count = 0;

  while (time_budget) {
    TestIter(count, body, params);
    ++count;
  }

  fmt::println("Executions checked: {}", count);
  ::twist::rt::thr::fault::Adversary()->PrintReport();
  std::cout.flush();
}

}  // namespace randomize
}  // namespace course::test::twist

#else

// just threads

namespace course::test::twist {
namespace randomize {

void Check(TestBody, Params) {
  WHEELS_PANIC("Random checking is not supported for this build");
}

}  // namespace randomize
}  // namespace course::test::twist

#endif
