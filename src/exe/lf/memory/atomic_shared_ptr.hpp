#pragma once

#include <twist/ed/std/atomic.hpp>

#include <twist/assist/memory.hpp>
#include <twist/trace/scope.hpp>

#include <cassert>
#include <cstdlib>  // std::abort

#include <exe/util/stamped_ptr.hpp>

//////////////////////////////////////////////////////////////////////

namespace exe::lf::memory {
namespace detail {

struct SplitCount {
  int32_t transient{0};
  int32_t strong{0};  // > 0
};

static_assert(sizeof(SplitCount) == sizeof(size_t),
              "AtomicSharedPtr supported");

using AtomicSplitCount = twist::ed::std::atomic<SplitCount>;

SplitCount FetchAddTransient(AtomicSplitCount& sc, int32_t add) {
  auto old_sc = sc.load();
  auto new_sc = old_sc;
  new_sc.transient += add;
  while (!sc.compare_exchange_weak(old_sc, new_sc)) {
    new_sc = old_sc;
    new_sc.transient += add;
  }
  return old_sc;
}

SplitCount FetchAddStrong(AtomicSplitCount& sc, int32_t add) {
  auto old_sc = sc.load();
  auto new_sc = old_sc;
  new_sc.strong += add;
  while (!sc.compare_exchange_weak(old_sc, new_sc)) {
    new_sc = old_sc;
    new_sc.strong += add;
  }
  return old_sc;
}

SplitCount FetchAddTransientAndStrong(AtomicSplitCount& sc,
                                      int32_t transient_add,
                                      int32_t strong_add) {
  auto old_sc = sc.load();
  auto new_sc = old_sc;
  new_sc.transient += transient_add;
  new_sc.strong += strong_add;
  while (!sc.compare_exchange_weak(old_sc, new_sc)) {
    new_sc = old_sc;
    new_sc.transient += transient_add;
    new_sc.strong += strong_add;
  }
  return old_sc;
}

template <typename T>
struct Block {
  template <typename... Args>
  explicit Block(Args&&... args)
      : value(std::forward<Args>(args)...) {
  }

  T value;
  AtomicSplitCount counter = SplitCount(0, 0);
};

}  // namespace detail

//////////////////////////////////////////////////////////////////////

template <typename T>
class AtomicSharedPtr;

template <typename T>
class SharedPtr;

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args);

template <typename T>
class SharedPtr {
  friend class AtomicSharedPtr<T>;

  template <typename U, typename... Args>
  friend SharedPtr<U> MakeShared(Args&&... args);

 public:
  SharedPtr()
      : block_(nullptr) {
  }

  SharedPtr(const SharedPtr<T>& that)
      : block_(that.block_) {
    if (block_ == nullptr) {
      return;
    }
    detail::FetchAddTransient(block_->counter, 1);  // this is fine
  }

  SharedPtr<T>& operator=(const SharedPtr<T>& that) {
    Reset();
    block_ = that.block_;
    if (block_ != nullptr) {
      detail::FetchAddTransient(block_->counter, 1);
    }
    return *this;
  }

  SharedPtr(SharedPtr<T>&& that)
      : block_(that.block_) {
    that.block_ = nullptr;
  }

  SharedPtr<T>& operator=(SharedPtr<T>&& that) {
    Reset();
    block_ = that.block_;
    that.block_ = nullptr;
    return *this;
  }

  T* operator->() const {
    return &(block_->value);
  }

  T& operator*() const {
    return block_->value;
  }

  explicit operator bool() const {
    return block_ != nullptr;
  }

  void Reset() {
    if (block_ == nullptr) {
      return;
    }
    auto sc_old = detail::FetchAddTransient(block_->counter, -1);
    sc_old.transient--;
    if (sc_old.transient == 0 && sc_old.strong == 0) {
      //      std::cout << "shared ptr: deleting addr " << block_ << std::endl;
      delete block_;
    }
    block_ = nullptr;
  }

  ~SharedPtr() {
    Reset();
  }

 private:
  explicit SharedPtr(detail::Block<T>* block)
      : block_(block) {
  }

 private:
  detail::Block<T>* block_{nullptr};
};

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
  auto block =
      std::make_unique<detail::Block<T>>(std::forward<Args>(args)...).release();
  //  std::cout << "created addr " << block << std::endl;
  detail::FetchAddTransient(block->counter, 1);
  return SharedPtr<T>(block);
}

//////////////////////////////////////////////////////////////////////

template <typename T>
class AtomicSharedPtr {
 public:
  // nullptr
  AtomicSharedPtr()
      : block_(StampedPtr<detail::Block<T>>(nullptr, 0)) {
  }

  ~AtomicSharedPtr() {
    Store(SharedPtr<T>());
  }

  AtomicSharedPtr(const AtomicSharedPtr& other) = delete;
  AtomicSharedPtr(AtomicSharedPtr&& other) = delete;
  AtomicSharedPtr operator=(const AtomicSharedPtr& other) = delete;
  AtomicSharedPtr operator=(AtomicSharedPtr&& other) = delete;

  SharedPtr<T> Load() {
    auto old_block = FetchIncrement(block_);
    old_block.stamp++;

    if (old_block.stamp > 10000) {
      TryUpdateTransient(old_block);
    }
    return SharedPtr<T>(old_block.raw_ptr);
  }

  void Store(SharedPtr<T> target) {
    if (target.block_ != nullptr) {
      detail::FetchAddStrong(target.block_->counter, 1);
    }
    auto block_old = block_.Exchange(StampedPtr(target.block_, 0));

    CleanupStampedBlock(block_old);
  }

  explicit operator SharedPtr<T>() {
    return Load();
  }

  bool CompareExchangeWeak(SharedPtr<T>& expected, SharedPtr<T> desired) {
    StampedPtr<detail::Block<T>> expected_sptr(expected.block_,
                                               block_.Load().stamp);

    if (desired.block_ != nullptr) {
      detail::FetchAddStrong(desired.block_->counter, 1);
    }

    auto was_expected_sptr = expected_sptr;

    if (!block_.CompareExchangeWeak(
            expected_sptr, StampedPtr<detail::Block<T>>(desired.block_, 0))) {
      if (desired.block_ != nullptr) {
        detail::FetchAddStrong(desired.block_->counter, -1);
      }
      expected = Load();
      return false;
    }

    CleanupStampedBlock(was_expected_sptr);
    return true;
  }

 private:
  // thread must own stamped block pointer to call this
  void CleanupStampedBlock(StampedPtr<detail::Block<T>> sp) {
    if (sp.raw_ptr == nullptr) {
      return;
    }
    auto sc = detail::FetchAddTransientAndStrong(sp->counter, sp.stamp, -1);
    sc.transient += sp.stamp;
    sc.strong -= 1;
    if (sc.transient == 0 && sc.strong == 0) {
      //      std::cout << "atomic shared ptr: deleting addr " << sp.raw_ptr <<
      //      std::endl;
      delete sp.raw_ptr;
    }
  }

  // caller must be sure, that at least one weak ptr of old_block will survive
  // through this call
  bool TryUpdateTransient(StampedPtr<detail::Block<T>> old_block) {
    if (old_block.raw_ptr == nullptr) {
      return false;
    }

    detail::FetchAddTransientAndStrong(old_block->counter, old_block.stamp, 1);
    auto was_old_block = old_block;

    if (!block_.CompareExchangeWeak(
            old_block, StampedPtr<detail::Block<T>>(old_block.raw_ptr, 0))) {
      detail::FetchAddTransientAndStrong(was_old_block->counter,
                                         -int32_t(was_old_block.stamp), -1);
      return false;
    }

    detail::FetchAddStrong(old_block->counter, -1);

    return true;
  }

 private:
  static constexpr int kCacheSyncFreq = 100;

  AtomicStampedPtr<detail::Block<T>> block_;
};

} // namespace exe::lf::memory
