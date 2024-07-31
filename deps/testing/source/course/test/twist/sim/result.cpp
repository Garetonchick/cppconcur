#include "result.hpp"

#include <fmt/core.h>

namespace course::test::twist {
namespace sim {

std::string FormatFailure(::twist::sim::Result result) {
  return fmt::format("Simulation failure ({}): {}", result.status, result.std_err);
}

}  // namespace sim
}  // namespace course::test::twist
