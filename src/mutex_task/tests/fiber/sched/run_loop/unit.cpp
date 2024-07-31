#include <exe/sched/run_loop.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>

#include <wheels/test/framework.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(Fibers) {
  SIMPLE_TEST(Go) {
    sched::RunLoop loop;

    fiber::Go(loop, []{});

    size_t tasks = loop.Run();
    ASSERT_EQ(tasks, 1);
  }

  SIMPLE_TEST(GoGroup) {
    sched::RunLoop loop;

    const size_t kFibers = 7;

    for (size_t i = 0; i < kFibers; ++i) {
      fiber::Go(loop, []{});
    }

    size_t tasks = loop.Run();
    ASSERT_EQ(tasks, kFibers);
  }

  SIMPLE_TEST(GoChild) {
    sched::RunLoop loop;

    bool flag = false;

    fiber::Go(loop, [&] {
      fiber::Go([&] {
        flag = true;
      });
    });

    ASSERT_TRUE(loop.RunNext());
    ASSERT_FALSE(flag);
    ASSERT_EQ(loop.Run(), 1);
    ASSERT_TRUE(flag);
  }

  SIMPLE_TEST(Yield) {
    sched::RunLoop loop;

    fiber::Go(loop, [] {
      fiber::Yield();
    });

    ASSERT_EQ(loop.Run(), 2);
  }

  SIMPLE_TEST(PingPong) {
    sched::RunLoop loop;

    int turn = 0;

    fiber::Go(loop, [&] {
      for (size_t i = 0; i < 3; ++i) {
        ASSERT_EQ(turn, 0);
        turn ^= 1;
        fiber::Yield();
      }
    });

    fiber::Go(loop, [&] {
      for (size_t j = 0; j < 3; ++j) {
        ASSERT_EQ(turn, 1);
        turn ^= 1;
        fiber::Yield();
      }
    });

    loop.Run();
  }

  SIMPLE_TEST(YieldGroup) {
    sched::RunLoop loop;

    const size_t kFibers = 3;
    const size_t kYields = 4;

    for (size_t i = 0; i < kFibers; ++i) {
      fiber::Go(loop, [] {
        for (size_t k = 0; k < kYields; ++k) {
          fiber::Yield();
        }
      });
    }

    size_t tasks = loop.Run();
    ASSERT_EQ(tasks, kFibers * (kYields + 1));
  }
}

RUN_ALL_TESTS()
