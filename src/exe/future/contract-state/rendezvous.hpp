#pragma once

#include <twist/ed/std/atomic.hpp>

namespace exe::future::state {

class RendezvousStateMachine {
  enum : uint32_t {
    Empty = 0,
    Produced = 1,
    Consumed = 2,
    Rendezvous = Produced | Consumed
  };

 public:
  // true means rendezvous
  bool Produce() {
    return state_.fetch_or(Produced) == Consumed;
  }

  // true means rendezvous
  bool Consume() {
    return state_.fetch_or(Consumed) == Produced;
  }

 private:
  twist::ed::std::atomic_uint32_t state_{Empty};
};

}  // namespace exe::future::state
