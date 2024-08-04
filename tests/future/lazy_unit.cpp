#include <exe/sched/thread_pool.hpp>
#include <exe/sched/run_loop.hpp>
#include <exe/sched/inline.hpp>

#include <exe/future/make/contract.hpp>
#include <exe/future/make/value.hpp>
#include <exe/future/make/submit.hpp>
#include <exe/future/make/just.hpp>

#include <exe/future/make/result/ok.hpp>
#include <exe/future/make/result/err.hpp>

#include <exe/future/combine/seq/map.hpp>
#include <exe/future/combine/seq/flatten.hpp>
#include <exe/future/combine/seq/flat_map.hpp>
#include <exe/future/combine/seq/via.hpp>
#include <exe/future/combine/seq/box.hpp>
#include <exe/future/combine/seq/start.hpp>

#include <exe/future/combine/seq/result/and_then.hpp>
#include <exe/future/combine/seq/result/or_else.hpp>
#include <exe/future/combine/seq/result/map_ok.hpp>

#include <exe/future/combine/concur/first.hpp>
#include <exe/future/combine/concur/all.hpp>

#include <exe/future/run/get.hpp>
#include <exe/future/run/detach.hpp>

#include <exe/result/make/ok.hpp>
#include <exe/result/make/err.hpp>

#include <wheels/test/framework.hpp>

#include <thread>
#include <chrono>

using namespace exe;

using namespace std::chrono_literals;

std::error_code TimeoutError() {
  return std::make_error_code(std::errc::timed_out);
}

std::error_code IoError() {
  return std::make_error_code(std::errc::io_error);
}

// clang-format off

TEST_SUITE(LazyFutures) {
  SIMPLE_TEST(JustMap) {
    bool run = false;

    auto f = future::Just()
                | future::Via(sched::Inline())
                | future::Map([&](Unit) {
                    run = true;
                    return unit;
                  });

    ASSERT_FALSE(run);

    [[maybe_unused]] Unit u = future::Get(std::move(f));
    ASSERT_TRUE(run);
  }

  SIMPLE_TEST(ContractMap) {
    auto [f, p] = future::Contract<Unit>();

    bool run = false;

    auto g = std::move(f)
             | future::Via(sched::Inline())
             | future::Map([&](Unit) {
                 run = true;
                 return unit;
               });

    std::move(p).Set(unit);

    ASSERT_FALSE(run);

    std::move(g) | future::Detach();

    ASSERT_TRUE(run);
  }

  SIMPLE_TEST(Submit) {
    sched::RunLoop loop;

    auto f = future::Submit(loop, [] {
               return 7;
             });

    ASSERT_TRUE(loop.IsEmpty());

    future::Detach(std::move(f));

    ASSERT_EQ(loop.Run(), 1);
  }

  SIMPLE_TEST(TryFuture) {
    sched::RunLoop loop;

    auto f = future::Just()
             | future::Via(loop)
             | future::Map([](Unit) {
                 return result::Ok(1);
               })
             | future::MapOk([](int v) {
                 return v + 1;
               })
             | future::AndThen([](int v) {
                 return future::Ok(v + 2);
               })
             | future::OrElse([](Error) {
                 return future::Ok(13);
               });

    ASSERT_TRUE(loop.IsEmpty());

    future::Detach(std::move(f));

    // ASSERT_EQ
    ASSERT_GE(loop.Run(), 2);
  }

  SIMPLE_TEST(Flatten) {
    sched::RunLoop loop;

    auto f = future::Value(1)
              | future::Via(loop)
              | future::Map([&loop](int v) {
                  return future::Submit(loop, [v] {
                    return v;
                  });
                })
              | future::Flatten();

    ASSERT_TRUE(loop.IsEmpty());

    future::Detach(std::move(f));

    ASSERT_TRUE(loop.Run() > 0);
  }

  SIMPLE_TEST(FlatMap) {
    sched::RunLoop loop;

    auto f = future::Just()
             | future::Via(loop)
             | future::FlatMap([&loop](Unit) {
                 return future::Submit(loop, [] {
                   return 11;
                 });
               });

    ASSERT_TRUE(loop.IsEmpty());

    future::Detach(std::move(f));

    ASSERT_TRUE(loop.Run() > 0);
  }

  SIMPLE_TEST(Box) {
    sched::RunLoop loop;

    future::BoxedFuture<int> f = future::Just()
                                  | future::Via(loop)
                                  | future::Map([](Unit) {
                                      return 7;
                                    })
                                  | future::Box();

    ASSERT_TRUE(loop.IsEmpty());

    future::Detach(std::move(f));

    ASSERT_TRUE(loop.Run() > 0);
  }

  SIMPLE_TEST(First) {
    sched::RunLoop loop;

    auto f = future::Just()
             | future::Via(loop)
             | future::Map([](Unit) {
                 return 1;
               });

    auto g = future::Just()
             | future::Via(loop)
             | future::Map([](Unit) {
                 return 2;
               });

    auto first = future::First(std::move(f), std::move(g));

    ASSERT_TRUE(loop.IsEmpty());

    future::Detach(std::move(first));

    ASSERT_EQ(loop.Run(), 2);
  }

  SIMPLE_TEST(Start) {
    sched::RunLoop loop;

    auto f = future::Just()
             | future::Via(loop)
             | future::Map([&](Unit) {
                 return 7;
               })
             | future::Start();

    ASSERT_TRUE(loop.NonEmpty());
    loop.Run();

    int v = future::Get(std::move(f));  // Ready

    ASSERT_EQ(v, 7);
  }

  SIMPLE_TEST(StartMap) {
    sched::RunLoop loop;

    auto f = future::Just()
             | future::Via(loop)
             | future::Map([&](Unit) {
                 return 7;
               })
             | future::Start()
             | future::Map([](int v) {
                 return v + 1;
               });

    {
      size_t tasks = loop.Run();
      ASSERT_EQ(tasks, 1);
    }

    future::Detach(std::move(f));

    {
      size_t tasks = loop.Run();
      ASSERT_EQ(tasks, 1);
    }
  }
}

RUN_ALL_TESTS()
