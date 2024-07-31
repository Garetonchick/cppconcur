#include <exe/sched/run_loop.hpp>
#include <exe/sched/task/submit.hpp>

#include <wheels/test/framework.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(RunLoop) {
  SIMPLE_TEST(JustWorks) {
    sched::RunLoop loop;
    
    size_t step = 0;

    ASSERT_FALSE(loop.NonEmpty());

    ASSERT_FALSE(loop.RunNext());
    ASSERT_EQ(loop.RunAtMost(99), 0);

    sched::task::Submit(loop, [&] {
      step = 1;
    });

    ASSERT_TRUE(loop.NonEmpty());

    ASSERT_EQ(step, 0);

    sched::task::Submit(loop, [&] {
      step = 2;
    });

    ASSERT_EQ(step, 0);

    ASSERT_TRUE(loop.RunNext());

    ASSERT_EQ(step, 1);

    ASSERT_TRUE(loop.NonEmpty());

    sched::task::Submit(loop, [&] {
      step = 3;
    });

    ASSERT_EQ(loop.RunAtMost(99), 2);
    ASSERT_EQ(step, 3);

    ASSERT_FALSE(loop.NonEmpty());
    ASSERT_FALSE(loop.RunNext());
  }

  SIMPLE_TEST(Empty) {
    sched::RunLoop loop;

    ASSERT_FALSE(loop.RunNext());
    ASSERT_EQ(loop.RunAtMost(7), 0);
    ASSERT_EQ(loop.Run(), 0);
  }

  void Countdown(sched::RunLoop& loop, size_t k) {
    if (k > 0) {
      sched::task::Submit(loop, [&loop, k] {
        Countdown(loop, k - 1);
      });
    }
  }

  SIMPLE_TEST(RunAtMost) {
    sched::RunLoop loop;

    Countdown(loop, 256);

    size_t tasks = 0;
    do {
      tasks += loop.RunAtMost(7);
    } while (loop.NonEmpty());

    fmt::println("{}", tasks);

    ASSERT_EQ(tasks, 256);
  }

  SIMPLE_TEST(RunAtMostNewTasks) {
    sched::RunLoop loop;

    sched::task::Submit(loop, [&](){
      sched::task::Submit(loop, [](){});
    });

    ASSERT_EQ(loop.RunAtMost(2), 2);
  }

  SIMPLE_TEST(Run) {
    sched::RunLoop loop;

    Countdown(loop, 117);

    ASSERT_EQ(loop.Run(), 117);
  }

  SIMPLE_TEST(RunTwice) {
    sched::RunLoop loop;

    Countdown(loop, 11);

    ASSERT_EQ(loop.Run(), 11);

    Countdown(loop, 7);

    ASSERT_EQ(loop.Run(), 7);
  }
}

RUN_ALL_TESTS()
