#pragma once

#include <function2/function2.hpp>

#include <cstdlib>  // std::abort
#include <iostream>

namespace exe::lf::memory::hazard {

using Deleter = fu2::function_view<void(void*)>;

template <typename T>
void DefaultDelete(void* obj) {
  delete (T*)obj;
}

inline void NopDelete(void*) {
}

struct RetiredPtr {
  void* object;
  Deleter deleter;

  RetiredPtr()
      : object(nullptr),
        deleter(NopDelete) {
  }

  RetiredPtr(void* obj, Deleter del)
      : object(obj),
        deleter(del) {
  }

  RetiredPtr(const RetiredPtr& o) = delete;
  RetiredPtr& operator=(const RetiredPtr& o) = delete;
  RetiredPtr(RetiredPtr&& o) {
    object = o.object;
    deleter = o.deleter;
    o.object = nullptr;
    o.deleter = NopDelete;
  };
  RetiredPtr& operator=(RetiredPtr&& o) {
    deleter(object);
    object = o.object;
    deleter = o.deleter;
    o.object = nullptr;
    o.deleter = NopDelete;
    return *this;
  }

  ~RetiredPtr() {
    deleter(object);
  }
};

template <typename T>
RetiredPtr Retired(T* obj) {
  return RetiredPtr(obj, DefaultDelete<T>);
}

}  // namespace exe::lf::memory::hazard
