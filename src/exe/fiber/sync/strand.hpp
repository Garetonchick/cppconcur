#pragma once

#include <exe/fiber/sched/suspend.hpp>
#include <exe/thread/spinlock.hpp>
#include <exe/sched/task/task.hpp>
#include <exe/fiber/core/fiber.hpp>

#include <cstdint>
#include <iostream>

namespace exe::fiber {

static constexpr size_t kWaiterDummy = 0xffffffffffffffff;
static constexpr size_t kWaiterMsk = 0x7fffffffffffffff;
static constexpr size_t kBusyBit = 0x8000000000000000;

class Strand {
 public:
  template <typename F>
  void Combine(F critical) {
    Waiter waiter(FiberHandle(&Fiber::Self()), std::move(critical));
    //    std::cout << "Enter combine" << std::endl;
    bool owner = false;
    WaiterBase* maybe_batch_head = nullptr;

    WaiterBase* maybe_nullptr = waiters_head_.load();
    if (maybe_nullptr == nullptr) {
      if (waiters_head_.compare_exchange_strong(maybe_nullptr, GetDummyWaiter(),
                                                std::memory_order_seq_cst)) {
        waiter.RunTask();
        ProcessBatch(nullptr);
        return;
      }
    }

    Awaiter awaiter([this, &waiter, &owner, &maybe_batch_head](auto) {
      WaiterBase* old_head = waiters_head_.load();
      while (true) {
        if (old_head == nullptr) {
          //          std::cout << "old_head == nullptr" << std::endl;
          if (waiters_head_.compare_exchange_strong(
                  old_head, GetDummyWaiter(), std::memory_order_seq_cst)) {
            owner = true;
            waiter.Schedule();
            return;
          }
        } else if (old_head == GetDummyWaiter()) {
          //          std::cout << "old_head == GetDummyWaiter()" << std::endl;
          waiter.SetNext(nullptr);
          if (waiters_head_.compare_exchange_strong(
                  old_head, AddBusyBit(&waiter), std::memory_order_seq_cst)) {
            return;
          }
        } else if (IsBusy(old_head)) {
          //          std::cout << "IsBusy(old_head)" << std::endl;
          waiter.SetNext(StripBusyBit(old_head));
          if (waiters_head_.compare_exchange_strong(
                  old_head, AddBusyBit(&waiter), std::memory_order_seq_cst)) {
            return;
          }
        } else {
          //          std::cout << "!IsBusy(old_head)" << std::endl;
          if (waiters_head_.compare_exchange_strong(
                  old_head, GetDummyWaiter(), std::memory_order_seq_cst)) {
            owner = true;
            maybe_batch_head = old_head;
            waiter.Schedule();
            return;
          }
        }
      }
    });
    Suspend(awaiter);
    //    std::cout << "Leave suspend" << std::endl;

    if (owner) {
      waiter.RunTask();
      ProcessBatch(maybe_batch_head);
      return;
    }

    WaiterBase* old_head = waiters_head_.load();

    if (old_head != nullptr && !IsBusy(old_head)) {
      if (waiters_head_.compare_exchange_strong(old_head, GetDummyWaiter(),
                                                std::memory_order_seq_cst)) {
        ProcessBatch(old_head);
        return;
      }
    }
    //    std::cout << "Leave combine" << std::endl;
  }

 private:
  class IWaiter {
   public:
    virtual ~IWaiter() = default;

    virtual void RunTask() noexcept = 0;
    virtual void Schedule() = 0;
  };
  class WaiterBase : public IWaiter,
                     public wheels::IntrusiveForwardListNode<WaiterBase> {};

  template <typename F>
  class Waiter : public WaiterBase {
   public:
    ~Waiter() = default;

    explicit Waiter(FiberHandle fiber, F&& f)
        : f_(std::move(f)),
          fiber_(fiber) {
    }

    void RunTask() noexcept override {
      f_();
    }

    void Schedule() override {
      fiber_.Schedule();
    }

   private:
    F f_;
    FiberHandle fiber_;
  };

 private:
  void ProcessBatch(WaiterBase* maybe_batch_head) {
    assert(maybe_batch_head != GetDummyWaiter());
    WaiterBase* batch_head = maybe_batch_head;

    if (batch_head == nullptr) {
      batch_head = waiters_head_.load();
      while (true) {
        if (batch_head == GetDummyWaiter()) {
          if (waiters_head_.compare_exchange_strong(
                  batch_head, nullptr, std::memory_order_seq_cst)) {
            return;
          }
        } else {
          if (waiters_head_.compare_exchange_strong(
                  batch_head, GetDummyWaiter(), std::memory_order_seq_cst)) {
            break;
          }
        }
      }
    }

    //      std::cout << batch_head << std::endl;
    batch_head = StripBusyBit(batch_head);

    wheels::IntrusiveForwardListNode<WaiterBase>* batch_head_node = batch_head;

    while (batch_head_node != nullptr) {
      auto waiter = batch_head_node->AsItem();
      batch_head_node = batch_head_node->Next();

      waiter->RunTask();

      if (batch_head_node == nullptr) {
        auto old_head = waiters_head_.load();
        while (true) {
          if (old_head == GetDummyWaiter()) {
            if (waiters_head_.compare_exchange_strong(
                    old_head, nullptr, std::memory_order_seq_cst)) {
              break;
            }
          } else {
            if (waiters_head_.compare_exchange_strong(
                    old_head, StripBusyBit(old_head),
                    std::memory_order_seq_cst)) {
              break;
            }
          }
        }
      }

      waiter->Schedule();
    }
  }

  static WaiterBase* AddBusyBit(WaiterBase* waiter) {
    return reinterpret_cast<WaiterBase*>(reinterpret_cast<size_t>(waiter) |
                                         kBusyBit);
  }

  static WaiterBase* StripBusyBit(WaiterBase* waiter) {
    assert(waiter != GetDummyWaiter());
    return reinterpret_cast<WaiterBase*>(reinterpret_cast<size_t>(waiter) &
                                         kWaiterMsk);
  }

  static WaiterBase* GetDummyWaiter() {
    return reinterpret_cast<WaiterBase*>(kWaiterDummy);
  }

  static bool IsBusy(WaiterBase* waiter) {
    return (reinterpret_cast<size_t>(waiter) & kBusyBit) != 0;
  }

 private:
  twist::ed::std::atomic<WaiterBase*> waiters_head_{nullptr};
};

}  // namespace exe::fiber
