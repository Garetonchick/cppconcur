#pragma once

#include "body.hpp"

#include <chrono>

namespace course::test::twist {

namespace stress {

struct Params {
  std::chrono::milliseconds time_budget;
};

void Test(TestBody body, Params params);

}  // namespace stress

}  // namespace course::test::twist
