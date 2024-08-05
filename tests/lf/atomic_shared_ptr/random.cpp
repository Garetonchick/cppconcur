#include <exe/lf/memory/atomic_shared_ptr.hpp>

#include <twist/ed/std/thread.hpp>

#include <course/test/twist.hpp>

#include <twist/assist/shared.hpp>

#include <twist/test/assert.hpp>
#include <twist/test/lock_free.hpp>

static_assert(twist::build::IsolatedSim());

template<typename T>
using AtomicSharedPtr = exe::lf::memory::AtomicSharedPtr<T>;

template<typename T>
using SharedPtr = exe::lf::memory::SharedPtr<T>;

using exe::lf::memory::MakeShared;

TEST_SUITE(AtomicSharedPtr) {
  TWIST_RANDOMIZE(Race, 5s) {
    using SharedInt = twist::assist::Shared<int>;

    // Init

    AtomicSharedPtr<SharedInt> asp;

    asp.Store(MakeShared<SharedInt>(1));

    // Concurrent

    twist::ed::std::thread reader([&] {
      twist::test::LockFreeScope lf;

      auto sp = asp.Load();

      twist::test::Progress();

      TWIST_TEST_ASSERT(sp, "Expected non-null pointer");

      int v = sp->Read();  // Happens-before checking

      TWIST_TEST_ASSERT(v == 1 || v == 2, "Unexpected value");
    });


    twist::ed::std::thread writer([&] {
      twist::test::LockFreeScope lf;

      asp.Store(MakeShared<SharedInt>(2));
      twist::test::Progress();
    });

    reader.join();
    writer.join();
  }
}

RUN_ALL_TESTS()
