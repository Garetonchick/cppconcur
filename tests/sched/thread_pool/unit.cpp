#include <exe/sched/thread_pool.hpp>
#include <exe/sched/task/submit.hpp>
#include <exe/thread/wait_group.hpp>

#include <wheels/test/framework.hpp>

#include <course/test/cpu.hpp>

#include <wheels/core/stop_watch.hpp>

#include <atomic>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;  // NOLINT

using namespace exe;  // NOLINT

TEST_SUITE(ThreadPool) {
  SIMPLE_TEST(WaitTask) {
    sched::ThreadPool pool{4};

    pool.Start();

    thread::WaitGroup wg;

    wg.Add(1);
    sched::task::Submit(pool, [&wg] {
      wg.Done();
    });

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(Wait) {
    sched::ThreadPool pool{4};

    pool.Start();

    thread::WaitGroup wg;

    wg.Add(1);
    sched::task::Submit(pool, [&wg] {
      std::this_thread::sleep_for(1s);
      wg.Done();
    });

    wg.Wait();
    pool.Stop();
  }

  SIMPLE_TEST(MultiWait) {
    sched::ThreadPool pool{1};

    pool.Start();

    for (size_t i = 0; i < 3; ++i) {
      thread::WaitGroup wg;

      wg.Add(1);

      sched::task::Submit(pool, [&wg] {
        std::this_thread::sleep_for(1s);
        wg.Done();
      });

      wg.Wait();
    }

    pool.Stop();
  }

  SIMPLE_TEST(ManyTasks) {
    sched::ThreadPool pool{4};

    pool.Start();

    static const size_t kTasks = 17;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kTasks; ++i) {
      wg.Add(1);
      sched::task::Submit(pool, [&wg] {
        wg.Done();
      });
    }

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(TwoPools) {
    sched::ThreadPool pool1{1};
    sched::ThreadPool pool2{1};

    pool1.Start();
    pool2.Start();

    wheels::StopWatch stop_watch;

    thread::WaitGroup wg1;
    wg1.Add(1);
    sched::task::Submit(pool1, [&] {
      std::this_thread::sleep_for(1s);
      wg1.Done();
    });

    thread::WaitGroup wg2;
    wg2.Add(1);
    sched::task::Submit(pool2, [&] {
      std::this_thread::sleep_for(1s);
      wg2.Done();
    });

    wg2.Wait();
    pool2.Stop();

    wg1.Wait();
    pool1.Stop();

    ASSERT_TRUE(stop_watch.Elapsed() < 1500ms);
  }

  SIMPLE_TEST(DoNotBurnCPU) {
    sched::ThreadPool pool{4};

    pool.Start();

    thread::WaitGroup wg;

    // Warmup
    for (size_t i = 0; i < 4; ++i) {
      wg.Add(1);
      sched::task::Submit(pool, [&wg] {
        std::this_thread::sleep_for(100ms);
        wg.Done();
      });
    }

    course::test::ProcessCPUTimer cpu_timer;

    std::this_thread::sleep_for(1s);

    wg.Wait();
    pool.Stop();

    ASSERT_TRUE(cpu_timer.Spent() < 100ms);
  }

  SIMPLE_TEST(Current) {
    sched::ThreadPool pool{1};
    pool.Start();

    ASSERT_EQ(sched::ThreadPool::Current(), nullptr);

    thread::WaitGroup wg;
    wg.Add(1);

    sched::task::Submit(pool, [&] {
      ASSERT_EQ(sched::ThreadPool::Current(), &pool);
      wg.Done();
    });

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(SubmitAfterWait) {
    sched::ThreadPool pool{4};

    pool.Start();

    thread::WaitGroup wg;

    wg.Add(1);
    sched::task::Submit(pool, [&] {
      std::this_thread::sleep_for(500ms);

      wg.Add(1);
      sched::task::Submit(pool, [&] {
        std::this_thread::sleep_for(500ms);
        wg.Done();
      });

      wg.Done();
    });

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(CrossSubmit) {
    sched::ThreadPool pool1{1};
    sched::ThreadPool pool2{1};

    pool1.Start();
    pool2.Start();

    thread::WaitGroup wg;
    wg.Add(1);

    sched::task::Submit(pool1, [&] {
      ASSERT_TRUE(sched::ThreadPool::Current() == &pool1);
      sched::task::Submit(pool2, [&] {
        ASSERT_TRUE(sched::ThreadPool::Current() == &pool2);
        wg.Done();
      });
    });

    wg.Wait();

    pool1.Stop();
    pool2.Stop();
  }

  TEST(TooManyThreads, wheels::test::TestOptions().TimeLimit(2s)) {
    sched::ThreadPool pool{3};

    pool.Start();

    thread::WaitGroup wg;

    for (size_t i = 0; i < 4; ++i) {
      wg.Add(1);
      sched::task::Submit(pool, [&wg] {
        std::this_thread::sleep_for(750ms);
        wg.Done();
      });
    }

    wheels::StopWatch stop_watch;

    wg.Wait();
    pool.Stop();

    ASSERT_TRUE(stop_watch.Elapsed() > 1s);
  }

  SIMPLE_TEST(TaskLifetime) {
    sched::ThreadPool pool{4};

    pool.Start();

    struct Widget {};

    auto w = std::make_shared<Widget>();

    thread::WaitGroup wg;
    for (int i = 0; i < 4; ++i) {
      wg.Add(1);
      sched::task::Submit(pool, [w, &wg] {
        wg.Done();
      });
    }

    std::this_thread::sleep_for(500ms);

    ASSERT_EQ(w.use_count(), 1)

    wg.Wait();

    pool.Stop();
  }

  SIMPLE_TEST(Racy) {
    sched::ThreadPool pool{4};

    pool.Start();

    std::atomic<size_t> racy_counter{0};

    static const size_t kTasks = 100500;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kTasks; ++i) {
      wg.Add(1);
      sched::task::Submit(pool, [&] {
        int old = racy_counter.load();
        racy_counter.store(old + 1);

        wg.Done();
      });
    }

    wg.Wait();

    pool.Stop();

    std::cout << "Racy counter value: " << racy_counter << std::endl;

    ASSERT_LE(racy_counter.load(), kTasks);
  }
}

RUN_ALL_TESTS()
