begin_task()

# Library
add_task_library(exe)

# Playground
add_task_playground(play)

# Tests

# Thread / WaitGroup
add_task_test(thread_wait_group_unit_tests tests/thread/wait_group/unit.cpp)
add_task_test(thread_wait_group_random_tests tests/thread/wait_group/random.cpp)
add_task_test(thread_wait_group_futex_tests tests/thread/wait_group/futex.cpp)
add_task_test(thread_wait_group_model_tests tests/thread/wait_group/model.cpp)
add_task_test(thread_wait_group_storage_tests tests/thread/wait_group/storage.cpp)

# Sched / ThreadPool
add_task_test(sched_thread_pool_unit_tests tests/sched/thread_pool/unit.cpp)
add_task_test(sched_thread_pool_random_tests tests/sched/thread_pool/random.cpp)
add_task_test(sched_thread_pool_stress_tests tests/sched/thread_pool/stress.cpp)

# Sched / RunLoop
add_task_test(sched_run_loop_unit_tests tests/sched/run_loop/unit.cpp)
add_task_test(sched_run_loop_sane_tests tests/sched/run_loop/sane.cpp)

# Fibers

# Fiber / Sched
add_task_test(fiber_sched_thread_pool_unit_tests tests/fiber/sched/thread_pool/unit.cpp)
add_task_test(fiber_sched_run_loop_unit_tests tests/fiber/sched/run_loop/unit.cpp)
add_task_test(fiber_sched_run_loop_sync_tests tests/fiber/sched/run_loop/sync.cpp)

# Fiber / Sync

# Fiber / Sync / Event

add_task_test(fiber_sync_event_unit_tests tests/fiber/sync/event/unit.cpp)
add_task_test(fiber_sync_event_random_tests tests/fiber/sync/event/random.cpp)
add_task_test(fiber_sync_event_stress_tests tests/fiber/sync/event/stress.cpp)

add_task_test(fiber_sync_event_storage_random_tests tests/fiber/sync/event/storage/random.cpp)
add_task_test(fiber_sync_event_storage_stress_tests tests/fiber/sync/event/storage/stress.cpp)

# Fiber / Sync / Mutex

add_task_test(fiber_sync_mutex_unit_tests tests/fiber/sync/mutex/unit.cpp)
add_task_test(fiber_sync_mutex_unit_tp_tests tests/fiber/sync/mutex/unit_tp.cpp)
add_task_test(fiber_sync_mutex_random_tests tests/fiber/sync/mutex/random.cpp)
add_task_test(fiber_sync_mutex_stress_tests tests/fiber/sync/mutex/stress.cpp)
add_task_test(fiber_sync_mutex_alloc_tests tests/fiber/sync/mutex/alloc.cpp)
add_task_test(fiber_sync_mutex_switch_tests tests/fiber/sync/mutex/switch.cpp)
add_task_test(fiber_sync_mutex_so_tests tests/fiber/sync/mutex/stack_overflow.cpp)

# Fiber / Sync / WaitGroup

add_task_test(fiber_sync_wait_group_unit_tests tests/fiber/sync/wait_group/unit.cpp)
add_task_test(fiber_sync_wait_group_random_tests tests/fiber/sync/wait_group/random.cpp)
add_task_test(fiber_sync_wait_group_stress_tests tests/fiber/sync/wait_group/stress.cpp)

add_task_test(fiber_sync_wait_group_storage_random_tests tests/fiber/sync/wait_group/storage/random.cpp)
add_task_test(fiber_sync_wait_group_storage_stress_tests tests/fiber/sync/wait_group/storage/stress.cpp)

end_task()
