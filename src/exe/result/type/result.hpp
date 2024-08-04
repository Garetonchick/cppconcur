#pragma once

#include <expected>

#include <exe/result/type/error.hpp>

namespace exe {

template <typename T>
using Result = std::expected<T, Error>;

}  // namespace exe
