#include <exe/sched/run_loop.hpp>

#include <wheels/test/framework.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(RunLoop) {
  SIMPLE_TEST(JustWorks) {
    sched::RunLoop loop;

    bool flag = false;

    class SetFlag : public sched::task::TaskBase {
     public:
      explicit SetFlag(bool& flag)
          : flag_(flag) {
      }

      void Run() noexcept override {
        flag_ = true;
      }

     private:
      bool& flag_;
    };

    {
      SetFlag set{flag};
      loop.Submit(&set);
      loop.RunNext();
    }

    ASSERT_TRUE(flag);
  }
}

RUN_ALL_TESTS()
