
#include <exe/sched/thread_pool.hpp>
#include <exe/sched/task/submit.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/build.hpp>

#include <twist/test/assert.hpp>

#include <twist/ed/std/thread.hpp>
#include <twist/ed/std/chrono.hpp>
#include <twist/ed/std/random.hpp>

#include <fmt/core.h>
#include <fmt/std.h>

#include <chrono>

using namespace exe;  // NOLINT
using namespace std::chrono_literals;  // NOLINT

// Parking + Load Balancing is hard

auto SteadyNow() {
  return twist::ed::std::chrono::steady_clock::now();
}

void Sleep1s() {
  twist::ed::std::this_thread::sleep_for(1s);
}

void ExternalSubmits() {
  static const size_t kThreads = 4;

  sched::ThreadPool pool{kThreads};
  pool.Start();

  course::test::TimeBudget time_budget;
  size_t iter = 0;

  while (time_budget) {
    ++iter;

    auto start = SteadyNow();

    size_t tasks = 1 + iter % kThreads;

    thread::WaitGroup wg;
    wg.Add(tasks);

    for (size_t i = 0; i < tasks; ++i) {
      sched::task::Submit(pool, [&] {
        Sleep1s();
        wg.Done();
      });
    }

    wg.Wait();

    auto elapsed = SteadyNow() - start;
    TWIST_TEST_ASSERT(elapsed < 1500ms, "Unbalanced");
  }

  pool.Stop();

  fmt::println("Iterations: {}", iter);
}

void InternalSubmits() {
  const size_t kThreads = 4;

  sched::ThreadPool pool{kThreads};
  pool.Start();

  course::test::TimeBudget time_budget;
  size_t iter = 0;

  while (time_budget) {
    ++iter;

    auto start = SteadyNow();

    thread::WaitGroup wg;

    size_t tasks = 1 + iter % kThreads;

    wg.Add(1);
    sched::task::Submit(pool, [&] {
      wg.Add(tasks);

      for (size_t i = 0; i < tasks; ++i) {
        sched::task::Submit(pool, [&] {
          Sleep1s();
          wg.Done();
        });
      }

      wg.Done();
    });

    wg.Wait();

    auto elapsed = SteadyNow() - start;
    TWIST_TEST_ASSERT(elapsed < 1500ms, "Unbalanced");
  }

  pool.Stop();

  fmt::println("Iterations: {}", iter);
}

void MixedSubmits() {
  twist::ed::std::random_device rd;
  std::mt19937 twister{rd()};

  sched::ThreadPool pool{6};
  pool.Start();

  course::test::TimeBudget time_budget;
  size_t iter = 0;

  while (time_budget) {
    ++iter;

    auto start = SteadyNow();

    size_t ext_tasks = 1 + twister() % 3;

    thread::WaitGroup wg;

    wg.Add(ext_tasks);

    for (size_t i = 0; i < ext_tasks; ++i) {
      bool spawn = (twister() % 2) == 0;

      sched::task::Submit(pool, [&pool, &wg, spawn] {
        if (spawn) {
          wg.Add(1);
          sched::task::Submit(pool, [&wg] {
            Sleep1s();
            wg.Done();
          });
        }

        Sleep1s();
        wg.Done();
      });
    }

    wg.Wait();

    auto elapsed = SteadyNow() - start;
    TWIST_TEST_ASSERT(elapsed < 1500ms, "Unbalanced");
  }

  pool.Stop();

  fmt::println("Iterations: {}", iter);
}

TEST_SUITE(LoadBalancing) {
  TWIST_STRESS_TEST(ExternalSubmits, 10s) {
    ExternalSubmits();
  }

  TWIST_STRESS_TEST(InternalSubmits, 10s) {
    InternalSubmits();
  }

  TWIST_STRESS_TEST(MixedSubmits, 10s) {
    MixedSubmits();
  }
}

RUN_ALL_TESTS()
