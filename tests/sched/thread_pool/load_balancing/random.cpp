#include <exe/sched/thread_pool.hpp>
#include <exe/sched/task/submit.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>

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

static_assert(twist::build::Sim());

// Parking + Load Balancing is hard

auto SteadyNow() {
  return twist::ed::std::chrono::steady_clock::now();
}

void Sleep1s() {
  twist::ed::std::this_thread::sleep_for(1s);
}

void ExternalSubmits() {
  twist::ed::std::random_device rd;
  std::mt19937 twister{rd()};

  const size_t kThreads = 4;

  sched::ThreadPool pool{kThreads};
  pool.Start();

  {
    auto start = SteadyNow();

    size_t tasks = 1 + twister() % kThreads;

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
}

void InternalSubmits() {
  twist::ed::std::random_device rd;
  std::mt19937 twister{rd()};

  const size_t kThreads = 4;

  sched::ThreadPool pool{kThreads};
  pool.Start();

  {
    auto start = SteadyNow();

    thread::WaitGroup wg;

    size_t tasks = 1 + twister() % kThreads;

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
}

void MixedSubmits() {
  twist::ed::std::random_device rd;
  std::mt19937 twister{rd()};

  sched::ThreadPool pool{6};
  pool.Start();

  {
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
}

TEST_SUITE(LoadBalancing) {
  TWIST_RANDOMIZE(ExternalSubmits, 10s) {
    ExternalSubmits();
  }

  TWIST_RANDOMIZE(InternalSubmits, 10s) {
    InternalSubmits();
  }

  TWIST_RANDOMIZE(MixedSubmits, 10s) {
    MixedSubmits();
  }
}

RUN_ALL_TESTS()
