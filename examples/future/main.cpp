#include <exe/sched/thread_pool.hpp>

#include <exe/future/make/contract.hpp>
#include <exe/future/make/submit.hpp>
#include <exe/future/make/value.hpp>
#include <exe/future/make/return.hpp>
#include <exe/future/make/just.hpp>

#include <exe/future/make/result/ok.hpp>
#include <exe/future/make/result/err.hpp>

#include <exe/future/combine/seq/map.hpp>
#include <exe/future/combine/seq/flatten.hpp>
#include <exe/future/combine/seq/flat_map.hpp>

#include <exe/future/combine/seq/via.hpp>

#include <exe/future/combine/seq/result/and_then.hpp>
#include <exe/future/combine/seq/result/or_else.hpp>
#include <exe/future/combine/seq/result/map_ok.hpp>

#include <exe/future/combine/concur/all.hpp>
#include <exe/future/combine/concur/first.hpp>

#include <exe/future/run/get.hpp>
#include <exe/future/run/detach.hpp>

#include <exe/future/syntax/both.hpp>
#include <exe/future/syntax/or.hpp>

#include <exe/result/make/ok.hpp>
#include <exe/result/make/err.hpp>

#include <exe/result/combine/map.hpp>
#include <exe/result/combine/and_then.hpp>
#include <exe/result/combine/or_else.hpp>

#include <fmt/core.h>

#include <cstdlib>  // std::abort
#include <thread>
#include <chrono>

using namespace exe;

Error Timeout() {
  return std::make_error_code(std::errc::timed_out);
}

template <typename T>
void PrintTypeOf(T&) {
  fmt::println("{}", __PRETTY_FUNCTION__);
}

int main() {
  sched::ThreadPool pool{4};
  pool.Start();

  {
    // Submit

    auto f = future::Submit(pool, [] {
      fmt::println("Running on thread pool");
      return 7;
    });

    int v = future::Get(std::move(f));

    fmt::println("Submit -> {}", v);
  }

  {
    // Value

    auto f = future::Value(42);

    auto v = future::Get(std::move(f));

    fmt::println("Value -> {}", v);
  }

  /*
   * Output(Mult(Add(Input(), 5), 7))
   *
   * Pipeline operator
   * x |> f(y) ~ f(x, y)
   *
   * Input() |> Add(5) |> Mult(7) |> Output()
   *
   */

  {
    // Submit + Map

    // Map: Future<T> -> (T -> U) -> Future<U>

    auto f = future::Submit(pool, [] {
               fmt::println("Running on thread pool");
               return 1;
             })
             | future::Map([](int v) {
                 return v + 1;
               });

    auto v = future::Get(std::move(f));

    fmt::println("Submit.Map -> {}", v);
  }

  {
    // Detach

    auto f = future::Submit(pool, [] {
      fmt::println("Running on thread pool");
      return unit;
    });

    future::Detach(std::move(f));

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  {
    // Submit = Just | Via | Map

    auto f = future::Just()
             | future::Via(pool)
             | future::Map([](Unit) {
                 fmt::println("Running on thread pool");
                 return unit;
               });

    PrintTypeOf(f);

    future::Get(std::move(f));
  }


  {
    // Pipeline

    auto f = future::Submit(pool, [] {
               return 1;
             })
             | future::Map([](int v) {
                 fmt::println("Inlined");
                 return v + 1;
               })
             | future::Via(pool)
             | future::Map([](int v) {
                 fmt::println("Running on thread pool");
                 return v + 1;
               });

    auto v = future::Get(std::move(f));

    fmt::println("Value.Map.Via.Map -> {}", v);
  }

  {
    // Flatten

    // Future<Future<T>> -> Future<T>

    future::Future<int> auto f = future::Submit(pool, [&] {
                                   return future::Submit(pool, [] {
                                     return 7;
                                   });
                                 })
                                 | future::Flatten();

    int v = future::Get(std::move(f));

    fmt::println("Flatten -> {}", v);
  }

  {
    // FlatMap: Future<T> -> (T -> Future<U>) -> Future<U>

    // A ; B
    // FlatMap | FlatMap

    future::Future<int> auto f = future::Submit(pool, [] {
                                   return 1;
                                 })
                                 | future::FlatMap([&pool](int v) {
                                     return future::Submit(pool, [v] {
                                       return v + 1;
                                     });
                                   });

    auto v = future::Get(std::move(f));

    fmt::println("FlatMap -> {}", v);
  }

  {
    // Monadic result

    auto r = result::Ok(1)
             | result::Map([](int v) {
                 return v * 2;
               })
             | result::AndThen([](int v) {
                 return result::Ok(v + 1);
               })
             | result::OrElse([](Error) {
                 return result::Ok(42);  // Fallback
               });

    fmt::println("Ok.AndThen.OrElse -> {}", r.value());
  }

  {
    // AndThen / OrElse
    // Asynchronous try-catch

    // using TryFuture<T> = Future<Result<T>>
    // AndThen: TryFuture<T> -> (T -> TryFuture<U>) -> TryFuture<U>
    // OrElse:  TryFuture<T> -> (Error -> TryFuture<T>) -> TryFuture<T>

    // MapOk: TryFuture<T> -> (T -> U) -> TryFuture<U>

    auto f = future::Submit(pool, [] {
               return result::Ok(1);
             })
             | future::AndThen([](int) {
                 return future::Err<int>(Timeout());
               })
             | future::AndThen([](int) {
                 std::abort();  // Unreachable
                 return future::Ok(1);
               })
             | future::OrElse([](std::error_code) {
                 return future::Ok(42);  // Fallback
               })
             | future::MapOk([](int v) {
                 return v + 1;
               });

    auto r = future::Get(std::move(f));

    fmt::println("AndThen.AndThen.OrElse.MapOk -> {}", *r);
  }

  {
    // Concurrent composition: First

    auto f = future::Submit(pool, [] {
      return 1;
    });

    auto g = future::Submit(pool, [] {
      return 2;
    });

    auto first = future::First(std::move(f), std::move(g));

    auto v = future::Get(std::move(first));

    fmt::println("First -> {}", v);
  }

  {
    // Concurrent composition: Both

    auto f = future::Submit(pool, [] {
      return 1;
    });

    auto g = future::Submit(pool, [] {
      return 2;
    });

    auto both = future::Both(std::move(f), std::move(g));

    auto [x, y] = future::Get(std::move(both));

    fmt::println("Both -> ({}, {})", x, y);
  }

  pool.Stop();

  return 0;
}
