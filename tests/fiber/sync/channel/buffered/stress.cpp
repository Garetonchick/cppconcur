#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/channel/buffered.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/ed/std/atomic.hpp>

#include <twist/assist/random.hpp>
#include <twist/test/assert.hpp>
#include <twist/test/message.hpp>
#include <twist/test/checksum.hpp>

using namespace exe;  // NOLINT

using TestValue = int64_t;
using TestMessage = twist::test::Message<TestValue>;

TEST_SUITE(StressBufferedChannel) {
  TWIST_STRESS_TEST(SingleProducerSingleConsumer, 10s) {
    sched::ThreadPool pool{4};
    pool.Start();

    fiber::BufferedChannel<TestMessage> chan{4};

    twist::test::CheckSum<TestValue> checksum;

    thread::WaitGroup wg;

    wg.Add(2);

    const TestValue kPoisonPill = -1;

    fiber::Go(pool, [&, chan] mutable {
      std::mt19937 twister{42};

      course::test::TimeBudget time_budget;

      while (time_budget) {
        auto v = static_cast<TestValue>(twister());
        chan.Send(TestMessage::New(v));
        checksum.Produce(v);
      }

      chan.Send(TestMessage::New(kPoisonPill));

      wg.Done();
    });

    fiber::Go(pool, [&, chan] mutable {
      while (true) {
        auto message = chan.Recv();
        TestValue v = message.Read();

        if (v != kPoisonPill) {
          checksum.Consume(v);
        } else {
          break;
        }
      }
      wg.Done();
    });

    wg.Wait();

    TWIST_TEST_ASSERT(checksum.Validate(), "Checksum mismatch");

    pool.Stop();
  }

  TWIST_STRESS_TEST(MultipleProducersMultipleConsumers, 10s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    const size_t kProducers = 3;
    const size_t kConsumers = 3;
    const size_t kBufferSize = 4;

    sched::ThreadPool pool{4};
    pool.Start();

    fiber::BufferedChannel<TestMessage> chan{kBufferSize};

    twist::ed::std::atomic_size_t producers_left = kProducers;

    twist::test::CheckSum<TestValue> checksum;

    thread::WaitGroup wg;
    wg.Add(kProducers + kConsumers);

    const TestValue kPoisonPill = -1;

    for (size_t i = 0; i < kProducers; ++i) {
      fiber::Go(pool, [&, i, chan] mutable {
        std::mt19937 twister{static_cast<uint32_t>(i)};

        course::test::TimeBudget time_budget;

        while (time_budget) {
          auto v = static_cast<TestValue>(twister());
          chan.Send(TestMessage::New(v));
          checksum.Produce(v);
        }

        if (producers_left.fetch_sub(1, std::memory_order::relaxed) == 1) {
          // Last sender
          for (size_t j = 0; j < kConsumers; ++j) {
            chan.Send(TestMessage::New(kPoisonPill));
          }
        }

        wg.Done();
      });
    }

    for (size_t j = 0; j < kConsumers; ++j) {
      fiber::Go(pool, [&, chan] mutable {
        while (true) {
          auto message = chan.Recv();
          TestValue v = message.Read();

          if (v != kPoisonPill) {
            checksum.Consume(v);
          } else {
            break;
          }
        }

        wg.Done();
      });
    }

    wg.Wait();

    TWIST_TEST_ASSERT(checksum.Validate(), "Checksum mismatch");

    pool.Stop();
  }
}

RUN_ALL_TESTS();
