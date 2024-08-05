#pragma once

#include "mutator.hpp"

namespace exe::lf::memory::hazard {

class Manager {
  friend class Mutator;

 public:
  static Manager& Get();

  Mutator MakeMutator();

  ~Manager();

 private:
};

}  // namespace exe::lf::memory::hazard
