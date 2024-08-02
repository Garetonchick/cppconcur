#pragma once

#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>
#include <wheels/intrusive/forward_list.hpp>

#include <optional>
#include <deque>

// Intrusive Unbounded blocking multi-producers/multi-consumers (MPMC) queue

template <typename T>
class IntrusiveUnboundedBlockingQueue {
  using Node = wheels::IntrusiveForwardListNode<T>;
  using Queue = IntrusiveUnboundedBlockingQueue<T>;

 public:
  void Push(Node* node) {
    std::lock_guard guard(mutex_);
    if (closed_) {
      return;
    }
    queue_.PushBack(node);
    if (waiters_ > 0) {
      CV.notify_one();
    }
  }

  std::optional<T*> Pop() {
    std::unique_lock lock(mutex_);

    ++waiters_;
    CV.wait(lock, [this]() {
      bool stop = !queue_.IsEmpty() or closed_;
      if (stop) {
        --waiters_;
      }
      return stop;
    });

    if (closed_ && queue_.IsEmpty()) {
      return std::nullopt;
    }

    return queue_.PopFront();
  }

  void Close() {
    std::lock_guard guard(mutex_);
    closed_ = true;
    CV.notify_all();
  }

 private:
  wheels::IntrusiveForwardList<T> queue_;
  twist::ed::std::condition_variable CV;  // NOLINT
  twist::ed::std::mutex mutex_;
  bool closed_{false};
  size_t waiters_ = 0;
};
