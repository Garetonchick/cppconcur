#include <exe/lf/lock_free_stack.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>
#include <course/test/counted.hpp>

#include <twist/test/assert.hpp>
#include <twist/test/checksum.hpp>
#include <twist/test/lock_free.hpp>
#include <twist/test/wg.hpp>

#include <fmt/core.h>

#include <atomic>
#include <random>

template<typename T>
using LockFreeStack = exe::lf::LockFreeStack<T>;

//////////////////////////////////////////////////////////////////////

struct Message: public course::test::Counted<Message> {
  uint64_t datum;

  explicit Message(uint64_t d)
      : datum(d) {
  }
};

//////////////////////////////////////////////////////////////////////

void StressTest(size_t threads, size_t batch_limit) {
  LockFreeStack<Message> stack;

  // Producers-consumers checksum
  twist::test::CheckSum<uint64_t> s;

  // Statistics
  std::atomic_size_t ops = 0;

  // Hard limit on number of alive objects
  const size_t kAliveMessagesHardLimit = 1024;

  // Run threads

  twist::test::WaitGroup wg;

  wg.Add(threads, [&](size_t index) {
    twist::test::LockFreeScope lf;

    std::mt19937_64 twister{index};

    course::test::TimeBudget budget;

    for (size_t iter = 0; budget; ++iter) {
      // Choose batch size
      size_t batch = 1 + iter % batch_limit;

      // Push

      for (size_t j = 0; j < batch; ++j) {
        auto d = twister();

        s.Produce(d);
        stack.Push(Message(d));
        twist::test::Progress();

        TWIST_TEST_ASSERT(Message::ObjectCount() < kAliveMessagesHardLimit,
                          "Too many alive messages");

        ops.fetch_add(1, std::memory_order::relaxed);
      }

      // Pop

      for (size_t j = 0; j < batch; ++j) {
        auto m = stack.TryPop();
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
  TWIST_TEST_ASSERT(!stack.TryPop(), "Expected empty stack");
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(StressLockFreeStack) {
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
