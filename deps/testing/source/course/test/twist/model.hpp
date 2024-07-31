#pragma once

#include "body.hpp"

#include <twist/build.hpp>

#include <chrono>
#include <optional>

namespace course::test::twist {

namespace model {

constexpr bool BuildSupported() {
  return ::twist::build::IsolatedSim();
}

constexpr std::chrono::seconds TimeBudget() {
  return std::chrono::minutes(5);
}

struct Params {
  std::optional<size_t> max_preemptions = std::nullopt;
  std::optional<size_t> max_steps = std::nullopt;
  bool spurious_wakeups = false;
  bool spurious_failures = false;
};

void Check(TestBody body, Params params);

}  // namespace model

}  // namespace course::test::twist
