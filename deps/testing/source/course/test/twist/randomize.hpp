#pragma once

#include "body.hpp"

#include <twist/build.hpp>

namespace course::test::twist {

namespace randomize {

constexpr bool BuildSupported() {
  return ::twist::build::Faulty() || ::twist::build::Sim();
}

std::chrono::milliseconds TestTimeLimit(std::chrono::milliseconds budget);

struct Params {
  std::chrono::milliseconds time_budget;
};

void Check(TestBody body, Params params);

}  // namespace randomize

}  // namespace course::test::twist
