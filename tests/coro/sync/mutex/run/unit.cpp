#include <exe/sched/run_loop.hpp>

#include <exe/coro/go.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/sync/mutex_flavor/run.hpp>

#include <wheels/test/framework.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(RunMutex) {
  SIMPLE_TEST(Lock) {
    sched::RunLoop loop;

    bool cs = false;

    auto locker = [&] -> coro::Go {
      co_await coro::JumpTo(loop);

      coro::mutex_flavor::RunMutex mutex;

      co_await mutex.Run([&] {
        cs = true;
      });
    };

    locker();

    loop.Run();

    ASSERT_TRUE(cs);
  }
}

RUN_ALL_TESTS()
