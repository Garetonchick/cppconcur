#include <exe/lf/lock_free_queue.hpp>

#include <twist/ed/std/thread.hpp>

#include <twist/assist/random.hpp>

#include <course/test/twist.hpp>

#include <twist/test/assert.hpp>
#include <twist/test/checksum.hpp>
#include <twist/test/lock_free.hpp>
#include <twist/test/message.hpp>
#include <twist/test/wg.hpp>

#include <random>

template<typename T>
using LockFreeQueue = exe::lf::LockFreeQueue<T>;

static_assert(twist::build::IsolatedSim());

using TestMessage = twist::test::Message<uint64_t>;

TEST_SUITE(RandomizeLockFreeQueue) {
  TWIST_RANDOMIZE(Chan, 10s) {
    twist::ed::std::random_device rd;
    twist::assist::Choice choice{rd};

    const size_t pushes = 1 + choice(5);

    LockFreeQueue<TestMessage> queue;

    twist::test::CheckSum<uint64_t> s;

    twist::ed::std::thread p([&, pushes] {
      twist::test::LockFreeScope lf;

      std::mt19937_64 twister{42};

      for (size_t i = 0; i < pushes; ++i) {
        auto d = twister();

        queue.Push(TestMessage::New(d));
        twist::test::Progress();
        s.Produce(d);
      }
    });

    {
      twist::test::LockFreeScope lf;

      size_t left = pushes;
      while (left > 0) {
        auto m = queue.TryPop();
        twist::test::Progress();

        if (m) {
          auto v = m->Read();
          s.Consume(v);  // Checksum
          --left;
        }
      }
    }

    p.join();

    TWIST_TEST_ASSERT(s.Validate(), "Checksum mismatch");
  }

  TWIST_RANDOMIZE(PushPop, 30s) {
    twist::ed::std::random_device rd;
    twist::assist::Choice choice{rd};

    const size_t threads = 2 + choice(4);
    const size_t pushes = 1 + choice(5);

    LockFreeQueue<TestMessage> queue;

    twist::test::CheckSum<uint64_t> s;

    twist::test::WaitGroup wg;

    wg.Add(threads, [&, pushes](size_t index) {
      twist::test::LockFreeScope lf;

      std::mt19937_64 twister{index};

      for (size_t i = 0; i < pushes; ++i) {
        {
          auto d = twister();

          queue.Push(TestMessage::New(d));

          // Report thread progress
          twist::test::Progress();

          // Checksum
          s.Produce(d);
        }

        {
          auto m = queue.TryPop();

          // Not empty
          TWIST_TEST_ASSERT(m, "Expected message");

          // Report thread progress
          twist::test::Progress();

          // Assuming m has value
          auto v = m->Read();
          s.Consume(v);
        }
      }

    });

    wg.Join();

    TWIST_TEST_ASSERT(s.Validate(), "Checksum mismatch");
  }
}

RUN_ALL_TESTS()
