#include <exe/lf/memory/atomic_shared_ptr.hpp>

#include <twist/ed/std/thread.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

template<typename T>
using AtomicSharedPtr = exe::lf::memory::AtomicSharedPtr<T>;

template<typename T>
using SharedPtr = exe::lf::memory::SharedPtr<T>;

using exe::lf::memory::MakeShared;

TEST_SUITE(StressAtomicSharedPtr) {
  void Race() {
    AtomicSharedPtr<std::string> asp;

    asp.Store(MakeShared<std::string>("Initial"));

    twist::ed::std::thread reader([&] {
      auto sp = asp.Load();
      ASSERT_TRUE(sp);
    });

    twist::ed::std::thread writer([&] {
      asp.Store(MakeShared<std::string>("Rewritten"));
    });

    reader.join();
    writer.join();
  }

  TWIST_STRESS_TEST(StoreLoad, 10s) {
    course::test::TimeBudget budget;

    while (budget) {
      Race();
    }
  }
}

RUN_ALL_TESTS()
