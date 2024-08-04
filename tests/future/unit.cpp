#include <exe/sched/thread_pool.hpp>
#include <exe/sched/run_loop.hpp>
#include <exe/sched/inline.hpp>

#include <exe/future/make/contract.hpp>
#include <exe/future/make/value.hpp>
#include <exe/future/make/submit.hpp>
#include <exe/future/make/return.hpp>
#include <exe/future/make/just.hpp>

#include <exe/future/make/result/ok.hpp>
#include <exe/future/make/result/err.hpp>

#include <exe/future/combine/seq/map.hpp>
#include <exe/future/combine/seq/flatten.hpp>
#include <exe/future/combine/seq/flat_map.hpp>
#include <exe/future/combine/seq/via.hpp>

#include <exe/future/combine/seq/result/map_ok.hpp>
#include <exe/future/combine/seq/result/and_then.hpp>
#include <exe/future/combine/seq/result/or_else.hpp>
#include <exe/future/combine/seq/result/as.hpp>

#include <exe/future/combine/concur/first.hpp>
#include <exe/future/combine/concur/all.hpp>

// #include <exe/future/combine/concur/result/first.hpp>
// #include <exe/future/combine/concur/result/all.hpp>

#include <exe/future/run/get.hpp>
#include <exe/future/run/detach.hpp>

#include <exe/result/make/ok.hpp>
#include <exe/result/make/err.hpp>

#include <wheels/test/framework.hpp>
#include <wheels/test/util/cpu_timer.hpp>

#include <fmt/core.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <tuple>

using namespace exe;

using namespace std::chrono_literals;

std::error_code TimeoutError() {
  return std::make_error_code(std::errc::timed_out);
}

std::error_code IoError() {
  return std::make_error_code(std::errc::io_error);
}

struct MoveOnly {
  MoveOnly() = default;
  MoveOnly(const MoveOnly&) = delete;
  MoveOnly(MoveOnly&&) {};
};

struct NonDefaultConstructible {
  NonDefaultConstructible(int) {};
};

TEST_SUITE(Futures) {
  SIMPLE_TEST(SubmitGet) {
    sched::ThreadPool pool{4};
    pool.Start();

    auto compute = future::Submit(pool, [] -> int {
      return 11;
    });

    int v = future::Get(std::move(compute));

    ASSERT_EQ(v, 11);

    pool.Stop();
  }

  SIMPLE_TEST(SubmitDetach) {
    sched::RunLoop loop;

    bool done = false;

    {
      auto f = future::Submit(loop, [&] {
        done = true;
        return unit;
      });

      future::Detach(std::move(f));
    }

    ASSERT_FALSE(done);

    loop.Run();

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(SubmitWait) {
    sched::ThreadPool pool{4};
    pool.Start();

    auto f = future::Submit(pool, [] {
      std::this_thread::sleep_for(1s);
      return 11;
    });

    wheels::ProcessCPUTimer timer;

    auto v = future::Get(std::move(f));

    ASSERT_TRUE(timer.Spent() < 100ms);

    ASSERT_EQ(v, 11);

    pool.Stop();
  }

  SIMPLE_TEST(Value) {
    auto f = future::Value(std::string("Hello"));
    auto s = future::Get(std::move(f));

    ASSERT_EQ(s, "Hello");
  }

  SIMPLE_TEST(Just) {
    auto j = future::Just();
    auto u = future::Get(std::move(j));
    ASSERT_EQ(u, unit);
  }

  SIMPLE_TEST(Map) {
    auto f = future::Value(1)
             | future::Via(sched::Inline())
             | future::Map([](int v) {
                 return v + 1;
               });

    auto v = future::Get(std::move(f));

    ASSERT_EQ(v, 2);
  }

  SIMPLE_TEST(Flatten) {
    sched::RunLoop loop;

    auto ff = future::Submit(loop, [&loop] {
      return future::Submit(loop, [] {
        return 7;
      });
    });

    future::Future<int> auto f = std::move(ff) | future::Flatten();

    bool ok = false;

    std::move(f) | future::Map([&ok](int v) {
      ASSERT_EQ(v, 7);
      ok = true;
      return unit;
    }) | future::Detach();

    ASSERT_FALSE(ok);

    loop.Run();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(FlatMap) {
    sched::RunLoop loop;

    future::Future<int> auto f = future::Submit(loop, [] {
                              return 23;
                            })
                            | future::FlatMap([&](int v) {
                                return future::Submit(loop, [v] {
                                  return v + 5;
                                });
                              })
                            | future::Map([](int v) {
                                return v + 1;
                              });

    bool ok = false;

    std::move(f)
        | future::Map([&](int v) {
            ASSERT_EQ(v, 29);
            ok = true;
            return unit;
          })
        | future::Detach();

    loop.Run();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(MapOk) {
    sched::ThreadPool pool{4};
    pool.Start();

    {
      auto f = future::Submit(pool, [] { return result::Ok(1); })
               | future::MapOk([](int v) {
                   return v + 1;
                 });

      auto r = future::Get(std::move(f));
      ASSERT_TRUE(r);
      ASSERT_EQ(*r, 2);
    }

    {
      auto f = future::Submit(pool, [] -> Result<int> {
                 return result::Err(TimeoutError());
               })
               | future::MapOk([](int) {
                   FAIL_TEST("Unreachable");
                   return unit;
                 });

      auto r = future::Get(std::move(f));
      ASSERT_FALSE(r);
    }

    pool.Stop();
  }

  SIMPLE_TEST(AndThen) {
    sched::ThreadPool pool{4};
    pool.Start();

    auto f = future::Submit(pool, [] {
               return result::Ok<std::string>("ok");
             })
             | future::AndThen([](auto s) {
                 return future::Ok(s + "!");
               })
             | future::AndThen([](auto s) {
                 return future::Ok(s + "!");
               });

    auto r = future::Get(std::move(f));

    ASSERT_TRUE(r);
    ASSERT_EQ(*r, "ok!!");

    pool.Stop();
  }

  SIMPLE_TEST(OrElse) {
    sched::ThreadPool pool{4};
    pool.Start();

    auto f = future::Submit(pool, [] -> Result<std::string> {
               return result::Err(IoError());
             })
             | future::OrElse([](Error e) {
                 const std::string kFallback = "fallback";
                 ASSERT_EQ(e, IoError());
                 return future::Ok(kFallback);
               });

    auto r = future::Get(std::move(f));

    ASSERT_TRUE(r);
    ASSERT_EQ(*r, "fallback");

    pool.Stop();
  }

  SIMPLE_TEST(TryPipeline) {
    auto f = future::Ready(3)
             | future::Via(sched::Inline())
             | future::Map([](int v) {
                 return v + 1;
               })
             | future::Map([](int v) {
                 return v + 2;
               })
             | future::AsTryFuture()
             | future::OrElse([](Error) {
                 FAIL_TEST("Unreachable");
                 return future::Ok(111);
               })
             | future::AndThen([](int /*v*/) {
                 return future::Err<int>(TimeoutError());
               })
             | future::MapOk([](int v) {
                 FAIL_TEST("Unreachable");
                 return v + 1;
               })
             | future::OrElse([](Error) {
                 return future::Ok(17);
               })
             | future::MapOk([](int v) {
                 return v + 1;
               });

    auto r = future::Get(std::move(f));

    ASSERT_TRUE(r);
    ASSERT_EQ(*r, 18);
  }

  SIMPLE_TEST(Via) {
    sched::RunLoop loop1;
    sched::RunLoop loop2;

    size_t steps = 0;

    future::Just()
        | future::Via(loop1)
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::Map([](Unit) {
            return unit;
          })
        | future::Via(loop2)
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::Via(loop1)
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::FlatMap([&](Unit) {
            ++steps;
            return future::Value(1);
          })
        | future::Map([&](int v) {
            ASSERT_EQ(v, 1);
            ++steps;
            return unit;
          })
        | future::Detach();

    ASSERT_EQ(loop1.Run(), 2);
    ASSERT_EQ(steps, 1);
    ASSERT_GE(loop2.Run(), 2);
    ASSERT_EQ(steps, 3);
    ASSERT_GE(loop1.Run(), 3);
    ASSERT_EQ(steps, 6);
  }

  SIMPLE_TEST(ContractValue) {
    auto [f, p] = future::Contract<std::string>();

    std::move(p).Set("Hi");
    std::string s = future::Get(std::move(f));

    ASSERT_EQ(s, "Hi");
  }

  SIMPLE_TEST(ContractDetach) {
    auto [f, p] = future::Contract<int>();

    future::Detach(std::move(f));
    std::move(p).Set(1);
  }

  SIMPLE_TEST(ContractMoveOnly) {
    auto [f, p] = future::Contract<MoveOnly>();

    std::move(p).Set(MoveOnly{});
    auto v = future::Get(std::move(f));

    WHEELS_UNUSED(v);
  }

  SIMPLE_TEST(ContractNonDefaultConstructible) {
    auto [f, p] = future::Contract<NonDefaultConstructible>();

    std::move(p).Set({128});
    future::Get(std::move(f));
  }

  SIMPLE_TEST(First1) {
    auto [f1, p1] = future::Contract<int>();
    auto [f2, p2] = future::Contract<int>();

    auto first = future::First(std::move(f1), std::move(f2));

    bool ok = false;

    auto check = std::move(first)
        | future::Via(sched::Inline())
        | future::Map([&ok](int v) {
            ASSERT_EQ(v, 1);
            ok = true;
            return unit;
          });

    future::Detach(std::move(check));

    std::move(p1).Set(1);

    ASSERT_TRUE(ok);

    std::move(p2).Set(2);
  }

  SIMPLE_TEST(First2) {
    auto [f1, p1] = future::Contract<int>();
    auto [f2, p2] = future::Contract<int>();

    auto first = future::First(std::move(f1), std::move(f2));

    bool ok = false;

    auto check = std::move(first)
        | future::Via(sched::Inline())
        | future::Map([&ok](int v) {
            ASSERT_EQ(v, 2);
            ok = true;
            return unit;
          });

    future::Detach(std::move(check));

    std::move(p2).Set(2);

    ASSERT_TRUE(ok);

    std::move(p1).Set(1);
  }

  SIMPLE_TEST(Both) {
    auto [f1, p1] = future::Contract<int>();
    auto [f2, p2] = future::Contract<int>();

    auto both = future::Both(std::move(f1), std::move(f2));

    bool ok = false;

    auto check = std::move(both)
        | future::Via(sched::Inline())
        | future::Map([&ok](auto tuple) {
            auto [x, y] = tuple;
            ASSERT_EQ(x, 1);
            ASSERT_EQ(y, 2);
            ok = true;
            return unit;
          });

    future::Detach(std::move(check));

    std::move(p2).Set(2);
    std::move(p1).Set(1);

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(DoNotWait1) {
    sched::ThreadPool pool{4};
    pool.Start();

    std::atomic_bool submit = false;

    auto f = future::Submit(pool,
                             [&] {
                               std::this_thread::sleep_for(1s);
                               submit = true;
                               return 11;
                             })
             | future::Map([](int v) {
                 return v + 1;
               });

    ASSERT_FALSE(submit);

    auto v = future::Get(std::move(f));

    ASSERT_TRUE(submit);
    ASSERT_EQ(v, 12);

    pool.Stop();
  }

  SIMPLE_TEST(DoNotWait2) {
    sched::ThreadPool pool{4};
    pool.Start();

    std::atomic_bool submit = false;

    auto f = future::Submit(pool,
                             [&] {
                               std::this_thread::sleep_for(1s);
                               submit = true;
                               return 31;
                             })
             | future::FlatMap([&](int v) {
                 return future::Submit(pool, [v] {
                   return v + 1;
                 });
               });

    ASSERT_FALSE(submit);

    auto v = future::Get(std::move(f));

    ASSERT_TRUE(submit);
    ASSERT_EQ(v, 32);

    pool.Stop();
  }

  SIMPLE_TEST(Inline1) {
    sched::RunLoop loop;

    bool ok = false;

    auto f = future::Just()
        | future::Via(loop)
        | future::Map([&](Unit) {
            ok = true;
            return unit;
          });

    future::Detach(std::move(f));

    size_t tasks = loop.Run();
    ASSERT_TRUE(ok);
    ASSERT_EQ(tasks, 1);
  }

  SIMPLE_TEST(Inline2) {
    sched::RunLoop loop;

    auto v = future::Get(future::Value(1) | future::Via(loop));

    ASSERT_EQ(v, 1);
  }

  SIMPLE_TEST(Inline3) {
    sched::RunLoop loop;

    bool flat_map = false;
    bool map1 = false;
    bool map2 = false;

    future::Just()
        | future::Via(loop)
        | future::FlatMap([&](Unit) {
            flat_map = true;
            return future::Value(unit);
          })
        | future::Map([&](Unit u) {
            map1 = true;
            return u;
          })
        | future::Map([&](Unit u) {
            map2 = true;
            return u;
          })
        | future::Detach();

    ASSERT_TRUE(loop.RunNext());
    ASSERT_TRUE(flat_map);
    ASSERT_FALSE(map1);

    ASSERT_TRUE(loop.RunNext());
    ASSERT_TRUE(map1);
    ASSERT_FALSE(map2);

    ASSERT_EQ(loop.Run(), 1);
    ASSERT_TRUE(map2);
  }

  SIMPLE_TEST(Inline4) {
    sched::RunLoop loop;

    auto f = future::Submit(loop, [&] {
      return future::Submit(loop, [] {
        return 19;
      });
    }) | future::Flatten();

    future::Detach(std::move(f));

    size_t tasks = loop.Run();
    ASSERT_EQ(tasks, 2);
  }
}

RUN_ALL_TESTS()
