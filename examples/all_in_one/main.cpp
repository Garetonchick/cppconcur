#include <exe/sched/thread_pool.hpp>
#include <exe/sched/run_loop.hpp>
#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/mutex.hpp>
#include <exe/fiber/sync/wait_group.hpp>
#include <exe/fiber/sync/event.hpp>
#include <exe/fiber/sched/yield.hpp>
#include <exe/fiber/sync/channel/buffered.hpp>
#include <exe/future/type/future.hpp>
#include <exe/future/type/boxed.hpp>
#include <exe/future/combine/seq/map.hpp>
#include <exe/future/make/submit.hpp>
#include <exe/future/combine/seq/via.hpp>
#include <exe/future/run/get.hpp>
#include <exe/coro/sched/jump.hpp>
#include <exe/coro/go.hpp>
#include <exe/coro/sync/mutex.hpp>
#include <exe/thread/wait_group.hpp>
#include <exe/util/defer.hpp>
#include <exe/lf/lock_free_queue.hpp>

#include <fmt/core.h>

#include <random>

using namespace exe;  // NOLINT

void Example() {
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
      auto comp = future::Submit(main_sched, [i, &reserve_sched, &dist, &rnd, &mutex] {
              thread::Event done;
              int val = 0;
              auto coroutine = [&] -> coro::Go { 
                co_await coro::JumpTo(reserve_sched);
                {
                  auto guard = co_await mutex.ScopedLock();
                  val = dist(rnd) ^ i;
                }
                done.Fire();
              };
              coroutine();
              done.Wait();
              return i;
            })
            | future::Map([](int v) {
                return v * 4;
              })
            | future::Via(reserve_sched)
            | future::Map([](int v) {
                return v + 1;
            });
      ch.Send(future::BoxedFuture<int>(std::move(comp)));
      fiber::Yield();
    }
    wg.Done();
  };

  auto consumer = [&wg, &ch, &results] {
    for(int i = 0; i < kNItems; ++i) {
      auto f = ch.Recv();
      int val = future::Get(std::move(f));
      results.Push(val);
    }
    wg.Done();
  };

  main_sched.Start();

  wg.Add(4);
  fiber::Go(main_sched, producer);
  fiber::Go(main_sched, producer);
  fiber::Go(main_sched, consumer);
  fiber::Go(main_sched, consumer);

  reserve_sched.Start();

  wg.Wait();
  main_sched.Stop();
  reserve_sched.Stop();

  fmt::println("Results:");
  while(auto maybe = results.TryPop()) {
    fmt::print("{} ", maybe.value());
  }
  fmt::println("");
}

int main() {
  Example();
  return 0;
}
