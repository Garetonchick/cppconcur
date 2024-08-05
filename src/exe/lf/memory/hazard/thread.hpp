#pragma once

#include "hazard_ptr.hpp"
#include "retired_ptr.hpp"
#include "limits.hpp"

namespace exe::lf::memory::hazard {

struct ThreadState {
  std::array<HazardPtr, kMaxHazardPtrs> slots;
  std::vector<RetiredPtr> retired;
};

}  // namespace exe::lf::memory::hazard
