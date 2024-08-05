#pragma once

#include <exe/sched/task/scheduler.hpp>

#include <cassert>
#include <coroutine>
#include <cstdlib>  // std::abort
#include <exception>

namespace exe::coro {

// Considered harmful

struct Go {
  struct Promise {
    Go get_return_object() {  // NOLINT
      return {};
    }

    std::suspend_never initial_suspend() {  // NOLINT
      return {};
    }

    struct FinalAwaiter {
      bool await_ready() noexcept {  // NOLINT
        return false;
      }

      void await_suspend(std::coroutine_handle<> h) noexcept {  // NOLINT
        h.destroy();
      }

      void await_resume() noexcept {  // NOLINT
      }
    };

    FinalAwaiter final_suspend() noexcept {  // NOLINT
      return {};
    }

    void return_void() {  // NOLINT
    }

    void set_exception(std::exception_ptr) {  // NOLINT
      std::abort();                           // Unhandled exception
    }

    void unhandled_exception() {  // NOLINT
      std::abort();               // Unhandled exception
    }

    void SetScheduler(sched::task::IScheduler* sched) {
      sched_ = sched;
    }

    sched::task::IScheduler* GetScheduler() {
      return sched_;
    }

   private:
    sched::task::IScheduler* sched_{nullptr};
  };
};

}  // namespace exe::coro

template <typename... Args>
struct std::coroutine_traits<exe::coro::Go, Args...> {
  using promise_type = exe::coro::Go::Promise;  // NOLINT
};
