#pragma once

#include <variant>

namespace exe {

using Unit = std::monostate;

inline constexpr Unit unit = Unit{};  // NOLINT

}  // namespace exe
