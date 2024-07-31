#pragma once

#include "twist/stress.hpp"
#include "twist/model.hpp"
#include "twist/randomize.hpp"

#include <chrono>

#include <wheels/test/framework.hpp>

////////////////////////////////////////////////////////////////////////////////

// Stress testing

#define TWIST_STRESS_TEST(name, time_budget) \
    void TwistStressTestBody##name();        \
    TEST(name, ::wheels::test::TestOptions().TimeLimit(time_budget)) {  \
      ::course::test::twist::stress::Test([] {                \
        TwistStressTestBody##name(); \
      }, {time_budget});                    \
    }                                     \
    void TwistStressTestBody##name()

////////////////////////////////////////////////////////////////////////////////

// Stateless model checking

#define TWIST_MODEL(name, params) \
    void TwistModelBody##name();        \
    TEST(name, ::wheels::test::TestOptions().TimeLimit(::course::test::twist::model::TimeBudget())) {                \
      static_assert(::course::test::twist::model::BuildSupported());                                   \
      ::course::test::twist::model::Check([] {                \
        TwistModelBody##name(); \
      }, params);                    \
    }                                     \
    void TwistModelBody##name()

////////////////////////////////////////////////////////////////////////////////

// Randomized checking

#define TWIST_RANDOMIZE(name, time_budget) \
  void TwistRandomCheckBody##name();        \
  TEST(name, ::wheels::test::TestOptions().TimeLimit(::course::test::twist::randomize::TestTimeLimit(time_budget))) { \
    static_assert(::course::test::twist::randomize::BuildSupported());              \
    ::course::test::twist::randomize::Check([] {                \
      TwistRandomCheckBody##name(); \
    }, {time_budget});                    \
  }                                     \
  void TwistRandomCheckBody##name()
