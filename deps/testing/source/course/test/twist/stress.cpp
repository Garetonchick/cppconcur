#include "stress.hpp"

#include <wheels/test/framework.hpp>

#if defined(__TWIST_SIM__)

// simulation

#include <twist/sim.hpp>

#if defined(__TWIST_SIM_ISOLATION__)

// matrix

#include "sim/result.hpp"

namespace course::test::twist {
namespace stress {

using RandomScheduler = ::twist::sim::sched::RandomScheduler;

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

void Test(TestBody body, Params) {
  RandomScheduler scheduler;

  ::twist::sim::Simulator simulator{&scheduler};
  auto result = simulator.Run(body);

  if (result.Failure()) {
    static const size_t kNotTooLarge = 256;

    if (result.iters < kNotTooLarge) {
      auto replay = TryRecord(body, scheduler);
      if (replay.digest != result.digest) {
        WHEELS_PANIC("Twist error: failed to record simulation replay");
      }

      ::twist::sim::Print(body, replay.schedule);

      FAIL_TEST(sim::FormatFailure(result));
    }
  } else {
    // Report
    fmt::println("Context switches injected: {}", scheduler.PreemptCount());

    return;  // Passed
  }
}

}  // namespace stress
}  // namespace course::test::twist

#else

// Simulation, maybe with sanitizers

namespace course::test::twist {
namespace stress {

void Test(TestBody body, Params) {
  ::twist::sim::sched::RandomScheduler scheduler;

  ::twist::sim::Simulator simulator{&scheduler};
  auto result = simulator.Run(body);

  if (result.Failure()) {
    // Impossible?
  } else {
    // Report
    fmt::println("Context switches injected: {}", scheduler.PreemptCount());

    return;  // Passed
  }
}

}  // namespace stress
}  // namespace course::test::twist

#endif

#elif defined(__TWIST_FAULTY__)

// threads + fault injection

#include <twist/rt/thr/fault/adversary/adversary.hpp>

#include <twist/rt/thr/logging/logging.hpp>

namespace course::test::twist {
namespace stress {

void Test(TestBody body, Params) {
  ::twist::rt::thr::fault::Adversary()->Reset();

  body();

  ::twist::rt::thr::fault::Adversary()->PrintReport();

  ::twist::rt::thr::log::FlushPendingLogMessages();
}

}  // namespace stress
}  // namespace course::test::twist

#else

// just threads

namespace course::test::twist {
namespace stress {

void Test(TestBody body, Params) {
  body();
}

}  // namespace stress
}  // namespace course::test::twist

#endif
