#pragma once

#include <cstddef>
#include <cstdlib>  // std::abort
#include <memory>

#include <exe/thread/spinlock.hpp>
#include <exe/fiber/sched/suspend.hpp>

namespace exe::fiber {

namespace detail {

// State shared between producers & consumers

template <typename T>
class BufferedChannelState {
 public:
  explicit BufferedChannelState(size_t capacity)
      : capacity_(capacity) {
    data_.reserve(capacity_ + 1);
  }

  ~BufferedChannelState() {
  }

  void Send(T value) {
    lock_.lock();

    while (IsFull()) {
      FiberHandle handle_copy;
      Awaiter awaiter([this, &handle_copy](auto handle) {
        handle_copy = handle;
        senders_.PushBack(&handle_copy);
        lock_.unlock();
      });
      Suspend(awaiter);
      lock_.lock();
    }

    if (data_.size() < capacity_ + 1) {
      data_.emplace_back(std::move(value));
    } else {
      Place(std::move(value), tail_);
    }
    tail_ = Next(tail_);

    if (recvs_.NonEmpty()) {
      auto waiter = recvs_.PopFront();
      waiter->Schedule(sched::task::SchedulerHint::Next);
    }

    lock_.unlock();
  }

  T Recv() {
    lock_.lock();

    while (IsEmpty()) {
      FiberHandle handle_copy;
      Awaiter awaiter([this, &handle_copy](auto handle) {
        handle_copy = handle;
        recvs_.PushBack(&handle_copy);
        lock_.unlock();
      });
      Suspend(awaiter);
      lock_.lock();
    }

    auto value = std::move(data_[head_]);
    head_ = Next(head_);

    if (senders_.NonEmpty()) {
      auto waiter = senders_.PopFront();
      waiter->Schedule(sched::task::SchedulerHint::Next);
    }

    lock_.unlock();
    return std::move(value);
  }

 private:
  size_t Next(size_t ptr) {
    return (ptr + 1) % (capacity_ + 1);
  }

  bool IsEmpty() {
    return tail_ == head_;
  }

  bool IsFull() {
    return Next(tail_) == head_;
  }

  void Place(T value, size_t pos) {
    data_[pos].~T();
    new (&data_[pos]) T(std::move(value));
  }

 private:
  const size_t capacity_;
  wheels::IntrusiveForwardList<FiberHandle> senders_;
  wheels::IntrusiveForwardList<FiberHandle> recvs_;

  size_t head_{0};
  size_t tail_{0};
  std::vector<T> data_;
  thread::SpinLock lock_;
};

}  // namespace detail

// Buffered MPMC Channel
// https://tour.golang.org/concurrency/3

template <typename T>
class BufferedChannel {
  using State = detail::BufferedChannelState<T>;

 public:
  // Bounded channel, `capacity` > 0
  explicit BufferedChannel(size_t capacity)
      : state_(std::make_shared<State>(capacity)) {
  }

  void Send(T value) {
    state_->Send(std::move(value));
  }

  T Recv() {
    return state_->Recv();
  }

 private:
  std::shared_ptr<State> state_;
};

}  // namespace exe::fiber
