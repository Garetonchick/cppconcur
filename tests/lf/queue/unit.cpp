#include <exe/lf/lock_free_queue.hpp>

#include <wheels/test/framework.hpp>

#include <string>

template<typename T>
using LockFreeQueue = exe::lf::LockFreeQueue<T>;

TEST_SUITE(LockFreeQueue) {
  SIMPLE_TEST(JustWorks) {
    LockFreeQueue<std::string> queue;

    queue.Push("Payload");
    auto item = queue.TryPop();
    ASSERT_TRUE(item);
    ASSERT_EQ(*item, "Payload");

    auto empty = queue.TryPop();
    ASSERT_FALSE(empty);
  }

  SIMPLE_TEST(FIFO) {
    LockFreeQueue<int> queue;

    queue.Push(1);
    queue.Push(2);
    queue.Push(3);

    ASSERT_EQ(*queue.TryPop(), 1);
    ASSERT_EQ(*queue.TryPop(), 2);
    ASSERT_EQ(*queue.TryPop(), 3);

    ASSERT_FALSE(queue.TryPop());
  }

  SIMPLE_TEST(Dtor) {
    {
      LockFreeQueue<std::string> queue;
      queue.Push("Hello");
      queue.Push("World");
    }
  }

  struct MoveOnly {
    MoveOnly() = default;

    // Movable
    MoveOnly(MoveOnly&&) = default;
    MoveOnly& operator=(MoveOnly&&) = default;

    // Non-copyable
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
  };

  SIMPLE_TEST(MoveOnlyValue) {
    LockFreeQueue<MoveOnly> queue;
    queue.Push({});
    ASSERT_TRUE(queue.TryPop());
  }

  struct NonDefaultConstructible {
    NonDefaultConstructible(int) {}
  };

  SIMPLE_TEST(NonDefaultConstructibleValue) {
    LockFreeQueue<NonDefaultConstructible> queue;
    queue.Push(NonDefaultConstructible{7});
    ASSERT_TRUE(queue.TryPop());
  }

  SIMPLE_TEST(TwoQueues) {
    LockFreeQueue<int> queue_1;
    LockFreeQueue<int> queue_2;

    queue_1.Push(3);
    queue_2.Push(11);
    ASSERT_EQ(*queue_1.TryPop(), 3);
    ASSERT_EQ(*queue_2.TryPop(), 11);
  }
}

RUN_ALL_TESTS()
