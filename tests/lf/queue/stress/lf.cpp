#include <exe/lf/lock_free_queue.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/test/assert.hpp>
#include <twist/test/checksum.hpp>
#include <twist/test/lock_free.hpp>
#include <twist/test/wg.hpp>

#include <fmt/core.h>

#include <atomic>
#include <random>

template<typename T>
using LockFreeQueue = exe::lf::LockFreeQueue<T>;

//////////////////////////////////////////////////////////////////////

struct Message {
  uint64_t datum;

  explicit Message(uint64_t d)
      : datum(d) {
  }
};

//////////////////////////////////////////////////////////////////////

void StressTest(size_t threads, size_t batch_size_limit) {
  LockFreeQueue<Message> queue;

  // Producers-consumers checksum
  twist::test::CheckSum<uint64_t> s;

  // Statistics
  std::atomic_size_t ops = 0;

  // Run threads

  twist::test::WaitGroup wg;

  wg.Add(threads, [&](size_t index) {
    twist::test::LockFreeScope lf;

    std::mt19937_64 twister{index};

    course::test::TimeBudget budget;

    for (size_t iter = 0; budget; ++iter) {
      // Choose batch size
      size_t batch = 1 + iter % batch_size_limit;

      // Push

      for (size_t j = 0; j < batch; ++j) {
        Message m{twister()};

        s.Produce(m.datum);
        queue.Push(std::move(m));
        twist::test::Progress();

        ops.fetch_add(1, std::memory_order::relaxed);
      }

      // Pop

      for (size_t j = 0; j < batch; ++j) {
        auto m = queue.TryPop();
        twist::test::Progress();
        ASSERT_TRUE(m);
        s.Consume(m->datum);
      }
    }
  });

  wg.Join();

  // Print statistics
  fmt::println("Operations = {}", ops.load());

  TWIST_TEST_ASSERT(s.Validate(), "Checksum mismatch");
  TWIST_TEST_ASSERT(!queue.TryPop(), "Expected empty queue");
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(StressLockFreeQueue) {
  TWIST_STRESS_TEST(Stress1, 5s) {
    StressTest(2, 2);
  }

  TWIST_STRESS_TEST(Stress2, 5s) {
    StressTest(5, 1);
  }

  TWIST_STRESS_TEST(Stress3, 5s) {
    StressTest(5, 3);
  }

  TWIST_STRESS_TEST(Stress4, 5s) {
    StressTest(5, 5);
  }
}

RUN_ALL_TESTS()
