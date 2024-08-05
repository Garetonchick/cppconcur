#pragma once

#include <exe/sched/tp/fast/thread_pool.hpp>
#include <exe/sched/tp/compute/thread_pool.hpp>

namespace exe::sched {

// Default thread pool implementation
using ThreadPool = tp::compute::ThreadPool;

}  // namespace exe::sched
