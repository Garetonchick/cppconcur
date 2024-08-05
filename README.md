# Cppconcur

Concurrency library for C++23 which has: 
* Thread pools (work-stealing `exe::sched::tp::fast::ThreadPool` and traditional `exe::sched::tp::compute::ThreadPool`)
* Fibers implemented using stackful coroutines (`exe::fiber::Go`) 
* Lazy futures (`exe::future::*`)
* Memory management for lock-free algorithms (`exe::lf::memory::hazard::HazardPtr`, `exe::lf::memory::AtomicSharedPtr`)
* Syncronization primitives such as Event, Mutex and WaitGroup for fibers (`exe::fiber::*`)
* Syncronization primitives such as Event, Mutex and WaitGroup for stackless coroutines (`exe::coro::*`)
* And more, checkout `examples` directory!

## All-in-one Example

```cpp
  fiber::BufferedChannel<future::BoxedFuture<int>> ch(1);
  lf::LockFreeQueue<int> results;
  thread::WaitGroup wg;
  sched::ThreadPool main_sched{4};
  sched::ThreadPool reserve_sched{2};
  coro::Mutex mutex;
  std::mt19937 rnd(4);
  std::uniform_int_distribution<int> dist(0, 100);

  constexpr int kNItems = 6;

  auto producer = [&] {
    for(int i = 0; i < kNItems; ++i) {
      // Create future and link it to main_sched thread pool
      auto comp = future::Submit(main_sched, [i, &reserve_sched, &dist, &rnd, &mutex] {
              thread::Event done;
              int val = 0;
              // Create coroutine lambda 
              auto coroutine = [&] -> coro::Go { 
                // Change coroutine's thread pool to reserve_sched 
                co_await coro::JumpTo(reserve_sched);
                {
                  // Critical section 
                  auto guard = co_await mutex.ScopedLock();
                  val = dist(rnd) ^ i;
                }
                done.Fire();
              };
              // Start coroutine 
              coroutine();
              // Wait coroutine to finish
              done.Wait();
              return i;
            })
            // Pipeline computations using '|' operator
            | future::Map([](int v) {
                return v * 4;
              })
            // Change thread pool to reserve_sched for the following computations 
            | future::Via(reserve_sched)
            | future::Map([](int v) {
                return v + 1;
            });
      // Send computation as boxed future to consumers via channel
      ch.Send(future::BoxedFuture<int>(std::move(comp)));
      // Reschedule this fiber 
      fiber::Yield();
    }
    wg.Done();
  };

  auto consumer = [&wg, &ch, &results] {
    for(int i = 0; i < kNItems; ++i) {
      // Receive future from channel 
      auto f = ch.Recv();
      // Synchronously wait for future's value
      int val = future::Get(std::move(f));
      // Push value into lock-free queue
      results.Push(val);
    }
    wg.Done();
  };

  main_sched.Start();

  wg.Add(4);
  // Add fiber tasks
  fiber::Go(main_sched, producer);
  fiber::Go(main_sched, producer);
  fiber::Go(main_sched, consumer);
  fiber::Go(main_sched, consumer);

  reserve_sched.Start();

  // Wait fibers to finish 
  wg.Wait();
  main_sched.Stop();
  reserve_sched.Stop();

  // Get items from results queue and print them out
  fmt::println("Results:");
  while(auto maybe = results.TryPop()) {
    fmt::print("{} ", maybe.value());
  }
  fmt::println("");
```

You can see full version of this example in `examples/all_in_one/main.cpp`.

## Build

Firstly generate build files:
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/clang++-17 -DCMAKE_C_COMPILER=/usr/bin/clang-17 ..
```

### Library
To build library run:
```
cd src
make
```

`libexe.a` file will contain the whole cppconcur library.

### Examples
To build examples run:
```
cd examples
make
```
You can find binaries in the `bin` directory.

### Tests
To build and run tests you don't need to run cmake yourself. Just use `mops` utility script 
as follows:
```
# Generate build files, needs to be done only once
./mops cmake
# Build and run all tests
./mops test
# Build and run tests from a specific pipeline
./mops test --config=pipelines/fiber/sched.json
# Build and run fiber_sched_thread_pool_unit_tests test target in Debug profile
./mops target fiber_sched_thread_pool_unit_tests Debug
```

See cmake-variants.yaml for all build profiles.
