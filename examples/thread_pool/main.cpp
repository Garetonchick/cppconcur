#include <exe/sched/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>

#include <exe/fiber/sync/mutex.hpp>
#include <exe/fiber/sync/wait_group.hpp>
#include <exe/fiber/sync/channel/buffered.hpp>

#include <exe/thread/wait_group.hpp>

#include <wheels/core/stop_watch.hpp>

#include <iostream>

using namespace exe;

//////////////////////////////////////////////////////////////////////

void WorkLoadContendedChannel() {
  struct Unit {};

  fiber::WaitGroup wg;

  fiber::BufferedChannel<Unit> chan{7};

  const size_t kProducers = 50;

  for (size_t k = 0; k < kProducers; ++k) {
    wg.Add(2);

    const size_t kSends = 100'500;

    // Producer
    fiber::Go([chan, &wg] mutable {
      for (size_t i = 0; i < kSends; ++i) {
        chan.Send({});
      }
      wg.Done();
    });

    // Symmetric consumer
    fiber::Go([chan, &wg] mutable {
      for (size_t i = 0; i < kSends; ++i) {
        chan.Recv();
      }
      wg.Done();
    });
  }

  wg.Wait();
}

//////////////////////////////////////////////////////////////////////

void WorkLoadChannels() {
  struct Unit {};

  fiber::WaitGroup wg;

  const size_t kPairs = 50;

  for (size_t k = 0; k < kPairs; ++k) {
    wg.Add(2);

    fiber::BufferedChannel<Unit> chan{7};

    const size_t kSends = 100'500;

    // Producer
    fiber::Go([chan, &wg] mutable {
      for (size_t i = 0; i < kSends; ++i) {
        chan.Send({});
      }
      wg.Done();
    });

    // Consumer
    fiber::Go([chan, &wg] mutable {
      for (size_t i = 0; i < kSends; ++i) {
        chan.Recv();
      }
      wg.Done();
    });
  }

  wg.Wait();
}

//////////////////////////////////////////////////////////////////////

void WorkLoadMutex() {
  fiber::WaitGroup wg1;

  for (size_t k = 0; k < 13; ++k) {
    wg1.Add(1);

    fiber::Go([&] {
      // Cluster

      fiber::WaitGroup wg2;

      size_t cs1 = 0;
      fiber::Mutex mutex1;

      size_t cs2 = 0;
      fiber::Mutex mutex2;

      for (size_t i = 0; i < 100; ++i) {
        wg2.Add(1);

        fiber::Go([&] {
          for (size_t j = 0; j < 65'536; ++j) {
            {
              std::lock_guard g(mutex1);
              ++cs1;
            }
            if (j % 17 == 0) {
              fiber::Yield();
            }
            {
              std::lock_guard g(mutex2);
              ++cs2;
            }
          }

          wg2.Done();
        });
      }

      wg2.Wait();

      wg1.Done();
    });
  }

  wg1.Wait();
}

//////////////////////////////////////////////////////////////////////

void WorkLoad() {
  wheels::StopWatch sw;

  sched::ThreadPool pool{4};
  pool.Start();

  thread::WaitGroup main;
  main.Add(1);

  fiber::Go(pool, [&main] {
//    WorkLoadMutex();
//    WorkLoadChannels();
    WorkLoadContendedChannel();
    main.Done();
  });

  main.Wait();

  pool.Stop();

  const auto elapsed = sw.Elapsed();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << "ms " << std::endl;
}

int main() {
  while (true) {
    WorkLoad();
  }
  return 0;
}
