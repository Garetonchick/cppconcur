#include "yield.hpp"

#include "suspend.hpp"

namespace exe::fiber {

void Yield() {
  Awaiter awaiter([](auto handle) {
    handle.Schedule();
  });
  Suspend(awaiter);
}

}  // namespace exe::fiber
