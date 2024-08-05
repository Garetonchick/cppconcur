#pragma once

#include <cstddef>

namespace exe::lf::memory::hazard {

static constexpr size_t kMaxHazardPtrs = 4;  // Arbitrary
static constexpr size_t kMaxThreads = 32;

}  // namespace exe::lf::memory::hazard
