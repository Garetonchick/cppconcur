#include <exe/sched/run_loop.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>

#include <exe/fiber/sync/channel/buffered.hpp>

#include <exe/thread/wait_group.hpp>

#include <wheels/test/framework.hpp>

using namespace exe; // NOLINT

TEST_SUITE(BufferedChannel) {
  SIMPLE_TEST(JustWorks) {
    sched::RunLoop loop;

    bool done = false;

    fiber::Go(loop, [&done] {
      fiber::BufferedChannel<int> chan{7};

      chan.Send(1);
      chan.Send(2);
      chan.Send(3);

      ASSERT_EQ(chan.Recv(), 1);
      ASSERT_EQ(chan.Recv(), 2);
      ASSERT_EQ(chan.Recv(), 3);

      done = true;
    });

    size_t tasks = loop.Run();

    ASSERT_EQ(tasks, 1);
    ASSERT_TRUE(done);
  }

  struct MoveOnly {
    MoveOnly() = default;

    // Non-movable
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;

    // Movable
    MoveOnly(MoveOnly&&) = default;
    MoveOnly& operator=(MoveOnly&&) = default;
  };

  SIMPLE_TEST(MoveOnlyItem) {
    sched::RunLoop loop;

    fiber::Go(loop, [] {
      fiber::BufferedChannel<MoveOnly> chan{3};

      chan.Send({});
      chan.Send({});

      chan.Recv();
      chan.Recv();
    });

    loop.Run();
  }

  struct NonDefaultConstructible {
    NonDefaultConstructible(int) {}
  };

  SIMPLE_TEST(NonDefaultConstructibleItem) {
    sched::RunLoop loop;

    fiber::Go(loop, [] {
      fiber::BufferedChannel<NonDefaultConstructible> chan{1};

      chan.Send({42});

      chan.Recv();
    });

    loop.Run();
  }

  SIMPLE_TEST(SuspendReceiver) {
    sched::RunLoop loop;

    fiber::BufferedChannel<int> chan{3};

    bool done = false;

    // Receiver
    fiber::Go(loop, [&done, chan] mutable {
      int value = chan.Recv();  // <-- Suspended
      ASSERT_EQ(value, 17);
      done = true;
    });

    {
      size_t tasks = loop.Run();
      ASSERT_TRUE(tasks < 3);
      ASSERT_FALSE(done);
    }

    // Sender
    fiber::Go(loop, [chan] mutable {
      chan.Send(17);  // <-- Resume suspended receiver
    });

    loop.Run();

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(SuspendSender) {
    sched::RunLoop loop;

    fiber::BufferedChannel<int> chan{2};

    int sent = 0;

    // Sender
    fiber::Go(loop, [&sent, chan] mutable {
      for (int v = 0; v < 3; ++v) {
        chan.Send(v);
        ++sent;
      }
    });

    {
      size_t tasks = loop.Run();
      ASSERT_TRUE(tasks < 5);
      ASSERT_EQ(sent, 2);
    }

    bool done = false;

    // Receiver
    fiber::Go(loop, [&done, chan] mutable {
      {
        int v = chan.Recv();  // <-- Resume suspended sender
        ASSERT_EQ(v, 0);
      }

      fiber::Yield();

      ASSERT_EQ(chan.Recv(), 1);
      ASSERT_EQ(chan.Recv(), 2);

      done = true;
    });

    // Recv + Complete sender
    loop.RunAtMost(2);

    ASSERT_EQ(sent, 3);

    // Complete receiver
    loop.Run();

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(Fifo) {
    sched::RunLoop loop;

    fiber::BufferedChannel<int> chan{7};

    const int kMessages = 128;

    fiber::Go(loop, [chan] mutable {
      for (int i = 0; i < kMessages; ++i) {
        chan.Send(i);

        if (i % 3 == 0) {
          fiber::Yield();
        }
      }
    });

    bool done = false;

    fiber::Go(loop, [&done, chan] mutable {
      for (int j = 0; j < kMessages; ++j) {
        ASSERT_EQ(chan.Recv(), j);

        if (j % 2 == 0) {
          fiber::Yield();
        }
      }
      done = true;
    });

    {
      size_t tasks = loop.Run();
      ASSERT_TRUE(tasks < 512);
    }

    ASSERT_TRUE(done);
  }
}

RUN_ALL_TESTS()
