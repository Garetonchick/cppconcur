#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/channel/buffered.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>

#include <twist/ed/std/atomic.hpp>

#include <twist/assist/random.hpp>
#include <twist/test/assert.hpp>
#include <twist/test/message.hpp>
#include <twist/test/checksum.hpp>

using namespace exe;  // NOLINT

static_assert(twist::build::IsolatedSim());

using TestValue = int64_t;
using TestMessage = twist::test::Message<TestValue>;

TEST_SUITE(RandomizeBufferedChannel) {
  TWIST_RANDOMIZE(SingleProducerSingleConsumer, 10s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    const size_t messages = 1 + choice(5);
    const size_t buffer = 1 + choice(messages);

    sched::ThreadPool pool{4};
    pool.Start();

    fiber::BufferedChannel<TestMessage> chan{buffer};

    twist::test::CheckSum<TestValue> checksum;

    thread::WaitGroup wg;

    wg.Add(2);

    fiber::Go(pool, [&, chan, messages] mutable {
      std::mt19937 twister{42};

      for (size_t i = 0; i < messages; ++i) {
        auto v = static_cast<TestValue>(twister());
        chan.Send(TestMessage::New(v));
        checksum.Produce(v);
      }
      wg.Done();
    });

    fiber::Go(pool, [&, chan, messages] mutable {
      for (size_t i = 0; i < messages; ++i) {
        auto message = chan.Recv();
        TestValue v = message.Read();
        checksum.Consume(v);
      }
      wg.Done();
    });

    wg.Wait();

    TWIST_TEST_ASSERT(checksum.Validate(), "Checksum mismatch");

    pool.Stop();
  }

  TWIST_RANDOMIZE(MultipleProducersMultipleConsumers, 10s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    const size_t producers = 1 + choice(3);
    const size_t consumers = 1 + choice(3);
    const size_t messages = 1 + choice(5);
    const size_t buffer = 1 + choice(messages);

    sched::ThreadPool pool{4};
    pool.Start();

    fiber::BufferedChannel<TestMessage> chan{buffer};

    twist::ed::std::atomic_int messages_left = static_cast<int>(messages);
    twist::ed::std::atomic_size_t producers_left = producers;

    twist::test::CheckSum<TestValue> checksum;

    thread::WaitGroup wg;
    wg.Add(producers + consumers);

    const TestValue kPoisonPill = -1;

    for (size_t i = 0; i < producers; ++i) {
      fiber::Go(pool, [&, i, chan] mutable {
        std::mt19937 twister{static_cast<uint32_t>(i)};

        while (messages_left.fetch_sub(1, std::memory_order::relaxed) > 0) {
          auto v = static_cast<TestValue>(twister());
          chan.Send(TestMessage::New(v));
          checksum.Produce(v);
        }

        if (producers_left.fetch_sub(1, std::memory_order::relaxed) == 1) {
          // Last sender
          for (size_t j = 0; j < consumers; ++j) {
            chan.Send(TestMessage::New(kPoisonPill));
          }
        }

        wg.Done();
      });
    }

    for (size_t j = 0; j < consumers; ++j) {
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
