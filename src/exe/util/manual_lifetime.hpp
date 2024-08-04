#pragma once

#include <array>
#include <cstddef>

namespace exe {

// Storage for value of type T with manual lifetime management

template <typename T>
class ManualLifetime {
 public:
  void Init(T val) {
    new (buf_.data()) T(std::move(val));
  }

  T& Access() {
    return *reinterpret_cast<T*>(buf_.data());
  }

  void Destroy() && {
    Access().~T();
  }

 private:
  alignas(alignof(T)) std::array<std::byte, sizeof(T)> buf_;
};

}  // namespace exe
