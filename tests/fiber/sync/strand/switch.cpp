#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/wait_group.hpp>
#include <exe/fiber/sync/strand.hpp>

#include <exe/thread/wait_group.hpp>

#include <twist/ed/std/thread.hpp>

#include <twist/sim.hpp>
#include <twist/test/assert.hpp>

#include <fmt/core.h>

#include <wheels/test/framework.hpp>

static_assert(twist::build::IsolatedSim());

TEST_SUITE(Strand) {

  class SharedState {
   public:
    void Access() {
      ++access_count_;

      auto thread_id = twist::ed::std::this_thread::get_id();
      if (thread_id != last_accessor_id_) {
        ++switch_count_;
        last_accessor_id_ = thread_id;
      }
    }

    size_t AccessCount() const {
      return access_count_;
    }

    size_t SwitchCount() const {
      return switch_count_;
    }

   private:
    size_t access_count_ = 0;
    twist::ed::std::thread::id last_accessor_id_;
    size_t switch_count_ = 0;
  };

  void Test() {
    using namespace exe;  // NOLINT

    sched::ThreadPool scheduler{4};
    scheduler.Start();

    SharedState state;

    thread::WaitGroup example;
    example.Add(1);

    const size_t kFibers = 256;
    const size_t kLocks = 1024;

    fiber::Go(scheduler, [&] {
      fiber::WaitGroup wg;

      fiber::Strand mutex;

      for (size_t i = 0; i < kFibers; ++i) {
        wg.Add(1);

        fiber::Go([&] {
          for (size_t j = 0; j < kLocks; ++j) {
            mutex.Combine([&] {
              state.Access();

              if (j % 3 == 0) {
                twist::ed::std::this_thread::yield();
              }
            });
          }

          wg.Done();
        });
      }

      wg.Wait();

      example.Done();
    });

    example.Wait();

    scheduler.Stop();

    size_t access_count = state.AccessCount();
    size_t switch_count = state.SwitchCount();

    fmt::println("# critical sections = {}", access_count);
    fmt::println("# switches = {}", switch_count);

    TWIST_TEST_ASSERT(access_count == kFibers * kLocks, "Missing critical sections");

    const size_t kSwitchHiThreshold = 2046;
    const size_t kSwitchLoThreshold = 32;

    TWIST_TEST_ASSERT(switch_count < kSwitchHiThreshold, "Too many thread switches");
    TWIST_TEST_ASSERT(switch_count > kSwitchLoThreshold, "Do not occupy thread");
  }

  TEST(SwitchCount, wheels::test::TestOptions().TimeLimit(std::chrono::seconds(10))) {
    twist::sim::sched::RandomScheduler scheduler;
    twist::sim::Simulator simulator{&scheduler};
    auto result = simulator.Run(Test);

    if (!result.Ok()) {
      fmt::println("{}: {}", result.status, result.std_err);
      FAIL_TEST(result.std_err);
    }
  }
}

RUN_ALL_TESTS()
