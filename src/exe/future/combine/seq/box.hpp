#pragma once

#include <exe/future/type/boxed.hpp>
#include <exe/future/syntax/pipe.hpp>
#include <exe/future/trait/value_of.hpp>
#include <exe/future/thunk/box.hpp>

#include <function2/function2.hpp>

namespace exe::future {

namespace pipe {

struct [[nodiscard]] Box {
  Box() = default;

  // Non-copyable
  Box(const Box&) = delete;

  template <SomeFuture InputFuture>
  BoxedFuture<trait::ValueOf<InputFuture>> Pipe(InputFuture f) {
    return thunk::Box<trait::ValueOf<InputFuture>>{std::move(f)};
  }
};

}  // namespace pipe

/*
 * Erase thunk type
 *
 */

inline auto Box() {
  return pipe::Box{};
}

}  // namespace exe::future
