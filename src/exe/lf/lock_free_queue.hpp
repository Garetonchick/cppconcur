#pragma once

#include "memory/hazard/manager.hpp"

#include <twist/ed/std/atomic.hpp>

#include <twist/assist/memory.hpp>
#include <twist/trace/scope.hpp>

#include <cstdlib>  // std::abort
#include <optional>
#include <iostream>

#include <exe/util/stamped_ptr.hpp>

// Michael-Scott unbounded lock-free queue
// https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf

namespace exe::lf {

namespace detail {
template <typename T>
struct Node {
  Node() = default;
  explicit Node(T val)
      : value(std::move(val)) {
  }

  std::optional<T> value;
  AtomicStampedPtr<Node> next{StampedPtr<Node>(nullptr, 0)};

  twist::ed::std::atomic_uint64_t try_del_count{0};
};
}  // namespace detail


template <typename T>
class LockFreePool {
  using Node = detail::Node<T>;

 public:
  void Push(Node* new_node) {
    auto old_top = top_.Load();
    auto old_next = new_node->next.Load();
    new_node->next.Store(StampedPtr(old_top.raw_ptr, old_next.stamp + 1));
    while (!top_.CompareExchangeWeak(old_top,
                                     StampedPtr(new_node, old_top.stamp + 1))) {
      new_node->next.Store(StampedPtr(old_top.raw_ptr, old_next.stamp + 1));
    }
  }

  std::optional<Node*> TryPop() {
    while (true) {
      auto curr_top = top_.Load();
      if (curr_top.raw_ptr == nullptr) {
        return std::nullopt;
      }
      auto next = curr_top->next.Load();

      if (top_.CompareExchangeWeak(
              curr_top, StampedPtr(next.raw_ptr, curr_top.stamp + 1))) {
        return curr_top.raw_ptr;
      }
    }
  }

  ~LockFreePool() {
    //    std::cout << "~LockFreePool()" << std::endl;
    auto cur = top_.Load();
    while (cur.raw_ptr != nullptr) {
      auto next = cur->next.Load();
      delete cur.raw_ptr;
      cur = next;
    }
  }

 private:
  AtomicStampedPtr<Node> top_{StampedPtr<Node>(nullptr, 0)};
};

template <typename T>
class LockFreeQueue {
  using Node = detail::Node<T>;

 public:
  LockFreeQueue() {
    auto dummy = new Node();
    dummy->try_del_count.store(1);
    //    first_node_ = StampedPtr(dummy, 0);
    head_.Store(StampedPtr(dummy, 0));
    tail_.Store(StampedPtr(dummy, 0));
  }

  void Push(T value) {
    //    std::lock_guard guard(lock_); // for testing

    Node* new_node = NewNode(std::move(value));

    StampedPtr<Node> old_tail(nullptr, 0);
    StampedPtr<Node> old_tail_next(nullptr, 0);

    while (true) {
      old_tail = tail_.Load();
      old_tail_next = old_tail->next.Load();

      if (old_tail != tail_.Load()) {  // consistency check
        continue;
      }

      if (old_tail_next.raw_ptr == nullptr) {
        if (old_tail->next.CompareExchangeWeak(
                old_tail_next, StampedPtr(new_node, old_tail_next.stamp + 1))) {
          break;
        }
      } else {
        tail_.CompareExchangeWeak(
            old_tail, StampedPtr(old_tail_next.raw_ptr,
                                 old_tail.stamp + 1));  // help advance tail
      }
    }
    tail_.CompareExchangeWeak(old_tail,
                              StampedPtr(new_node, old_tail.stamp + 1));
  }

  std::optional<T> TryPop() {
    //    std::lock_guard guard(lock_); // for testing

    StampedPtr<Node> head_next(nullptr, 0);
    while (true) {
      StampedPtr<Node> head_old = head_.Load();
      head_next = head_old->next.Load();
      StampedPtr<Node> tail_old = tail_.Load();

      if (head_.Load() != head_old) {  // consistency check
        continue;
      }

      if (head_next.raw_ptr == nullptr) {
        if (head_old.raw_ptr != tail_old.raw_ptr) {
          continue;
        }
        return std::nullopt;
      }

      if (head_old.raw_ptr != tail_old.raw_ptr) {
        if (head_.CompareExchangeWeak(
                head_old, StampedPtr(head_next.raw_ptr, head_old.stamp + 1))) {
          if (head_old->try_del_count.fetch_add(1) % 2 == 1) {
            ToPool(head_old.raw_ptr);
          }
          break;
        }
        continue;
      }

      tail_.CompareExchangeWeak(
          tail_old, StampedPtr(head_next.raw_ptr,
                               tail_old.stamp + 1));  // help advance tail
    }

    auto val = std::move(head_next->value);

    if (head_next->try_del_count.fetch_add(1) % 2 == 1) {
      ToPool(head_next.raw_ptr);
    }

    return val;
  }

  ~LockFreeQueue() {
    //    std::cout << "~LockFreeQueue()" << std::endl;
    auto cur = head_.Load();
    while (cur.raw_ptr != nullptr) {
      auto next = cur->next.Load();
      delete cur.raw_ptr;
      cur = next;
    }
  }

 private:
  void ToPool(Node* node) {
    pool_.Push(node);
  }

  Node* NewNode(T value) {
    auto maybe_new_node = pool_.TryPop();
    if (maybe_new_node.has_value()) {
      Node* new_node = maybe_new_node.value();
      new_node->value.~optional<T>();
      new (&new_node->value) std::optional<T>(std::move(value));
      auto old_next = new_node->next.Load();
      new_node->next.Store(StampedPtr<Node>(nullptr, old_next.stamp + 1));
      return new_node;
    }
    return new Node(std::move(value));
  }

 private:
  AtomicStampedPtr<Node> head_{StampedPtr<Node>(nullptr, 0)};
  AtomicStampedPtr<Node> tail_{StampedPtr<Node>(nullptr, 0)};
  LockFreePool<T> pool_;
  //  exe::thread::SpinLock lock_;
  //  StampedPtr<Node> first_node_{nullptr, 0};
};

} // namespace lf
