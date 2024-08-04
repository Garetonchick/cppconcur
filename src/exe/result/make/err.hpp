#pragma once

#include <exe/result/type/result.hpp>

namespace exe::result {

inline auto Err(Error error) {
  return std::unexpected(error);
}

}  // namespace exe::result
