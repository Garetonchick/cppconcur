#pragma once

#include <atomic>

#include <twist/assist/assert.hpp>

namespace course::test {

template <typename T>
struct Counted {
  inline static std::atomic_size_t count = 0;

  Counted() {
    IncrementCount();
  }

  Counted(const Counted&) {
    IncrementCount();
  }

  Counted(Counted&&) {
    IncrementCount();
  }

  Counted& operator=(const Counted&) {
    // No-op
  }

  Counted& operator=(Counted&&) {
    // No-op
  }

  ~Counted() {
    DecrementCount();
  }

  static size_t ObjectCount() {
    return count.load();
  }

 private:
  static void IncrementCount() {
    count.fetch_add(1, std::memory_order::relaxed);
  }

  static void DecrementCount() {
    count.fetch_sub(1, std::memory_order::relaxed);
  }
};

}  // namespace course::test
