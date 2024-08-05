#include "manager.hpp"

#include <twist/ed/static/var.hpp>
#include <twist/ed/static/thread_local/var.hpp>

#include <algorithm>
#include <iostream>
#include <set>

namespace exe::lf::memory::hazard {

template <typename T>
struct AtomicPointer : public twist::ed::std::atomic<T*> {
  AtomicPointer()
      : twist::ed::std::atomic<T*>(nullptr) {
  }
};

template <typename T>
struct AtomicNumber : public twist::ed::std::atomic<T> {
  AtomicNumber()
      : twist::ed::std::atomic<T>(0) {
  }
};

struct ThreadIdx {
  int64_t thread_idx{-1};
};

using ThreadStatesArray = std::array<ThreadState, kMaxThreads>;

// TWISTED_STATIC_THREAD_LOCAL(std::vector<void*>, hazards_buf);
TWISTED_STATIC_THREAD_LOCAL(ThreadIdx, thread_idx);

TWISTED_STATIC(AtomicNumber<size_t>, thread_state_next_idx);
TWISTED_STATIC(ThreadStatesArray, thread_states);

Manager& Manager::Get() {
  TWISTED_STATIC(Manager, instance);
  return *instance;
}

Mutator Manager::MakeMutator() {
  if (thread_idx->thread_idx == -1) {
    thread_idx->thread_idx = thread_state_next_idx->fetch_add(1);
    if (size_t(thread_idx->thread_idx) >= kMaxThreads) {
      std::cout << "Number of threads has exceeded kMaxThreads" << std::endl;
      std::abort();
    }
  }
  return Mutator(this, &(*thread_states)[thread_idx->thread_idx]);
}

Manager::~Manager() {
}

void Mutator::Scan() {
  //  size_t n_threads = thread_state_next_idx->load();
  //  std::vector<void*> hh;
  //  std::vector<void*>* hazards_buf = &hh;
  //  hazards_buf->reserve(kMaxThreads * kMaxHazardPtrs);
  //  hazards_buf->clear();
  std::set<void*> hazards;

  for (size_t i = 0; i < kMaxThreads; ++i) {
    auto& slots = (*thread_states)[i].slots;
    for (size_t j = 0; j < kMaxHazardPtrs; ++j) {
      void* obj = slots[j].Get();
      if (obj != nullptr) {
        //        hazards_buf->push_back(obj);
        hazards.insert(obj);
      }
    }
  }

  //  std::sort(hazards_buf->begin(), hazards_buf->end());

  auto& retired = thread_->retired;
  for (int64_t i = 0; i < int64_t(retired.size()); ++i) {
    //    auto it = std::lower_bound(hazards_buf->begin(), hazards_buf->end(),
    //                               retired[i], [](const void* obj, const
    //                               RetiredPtr& r){return obj < r.object;});
    //    if(it == hazards_buf->end() || *it != retired[i].object) {
    if (!hazards.contains(retired[i].object)) {
      std::swap(retired[i], retired.back());
      //      std::cout << "free " << retired.back().object << std::endl;
      retired.pop_back();
      --i;
    }
  }
}

void Mutator::CleanUpImpl() {
  auto& retired = thread_->retired;
  retired.clear();
}

}  // namespace exe::lf::memory::hazard
