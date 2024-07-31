#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>

#include <twist/ed/std/thread.hpp>

#include <twist/assist/shared.hpp>
#include <twist/test/assert.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(WaitGroup) {

  /*
   * Equivalent to:
   *
   * ThreadPool pool{4};
   * pool.Start();
   *
   * {
   *   WaitGroup wg;
   *
   *   wg.Add(1);
   *   pool.Submit([&wg] {
   *     wg.Done();
   *   });
   *
   *   wg.Wait();
   * }  // ~WaitGroup
   *
   * pool.Stop();
   */

  TWIST_RANDOMIZE(Storage, 5s) {
    auto* wg = new thread::WaitGroup{};

    wg->Add(1);
    twist::ed::std::thread t([&wg] {
      wg->Done();
    });

    wg->Wait();
    delete wg;

    t.join();
  }

  TWIST_RANDOMIZE(TwoWaits, 5s) {
    auto* wg1 = new thread::WaitGroup();
    auto* wg2 = new thread::WaitGroup();

    twist::assist::Shared<bool> f1{false};
    twist::assist::Shared<bool> f2{false};

    wg1->Add(1);
    wg2->Add(1);

    twist::ed::std::thread t1([&] {
      f1.Write(true);
      wg1->Done();
    });

    twist::ed::std::thread t2([&] {
      wg1->Wait();
      f2.Write(true);
      wg2->Done();
    });

    wg1->Wait();
    bool r1 = f1.Read();

    wg2->Wait();
    bool r2 = f2.Read();

    TWIST_TEST_ASSERT(r1 && r2, "Unfinished work");

    delete wg1;
    delete wg2;

    t1.join();
    t2.join();
  }
}

RUN_ALL_TESTS()
