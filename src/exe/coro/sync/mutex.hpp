#pragma once

#include "mutex_flavor/scoped_lock.hpp"

namespace exe::coro {

// Default
using Mutex = mutex_flavor::ScopedLockMutex;

}  // namespace exe::coro
