#pragma once

#include "start.hpp"

namespace exe::future {

/*
 * Force thunk evaluation
 *
 * Synonym for future::Start
 *
 */

inline auto Force() {
  return Start();
}

}  // namespace exe::future
