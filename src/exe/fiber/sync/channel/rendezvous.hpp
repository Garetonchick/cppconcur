#pragma once

#include <cstddef>
#include <cstdlib>  // std::abort
#include <memory>

namespace exe::fiber {

namespace detail {

// State shared between producers & consumers

template <typename T>
class RendezvousChannelState {
 public:
  RendezvousChannelState() = default;

  ~RendezvousChannelState() {
    // Mop implemented
  }

  void Send(T /*value*/) {
    std::abort();  // Mop implemented
  }

  T Recv() {
    std::abort();  // Mop implemented
  }

 private:
  // ???
};

}  // namespace detail

// Rendezvous MPMC Channel
// https://tour.golang.org/concurrency/2

template <typename T>
class RendezvousChannel {
  using State = detail::RendezvousChannelState<T>;

 public:
  RendezvousChannel()
      : state_(std::make_shared<State>()) {
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
