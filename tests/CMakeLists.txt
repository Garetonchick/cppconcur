# Thread / WaitGroup
add_concur_test(thread_wait_group_unit_tests thread/wait_group/unit.cpp)
add_concur_test(thread_wait_group_random_tests thread/wait_group/random.cpp)
add_concur_test(thread_wait_group_futex_tests thread/wait_group/futex.cpp)
add_concur_test(thread_wait_group_model_tests thread/wait_group/model.cpp)
add_concur_test(thread_wait_group_storage_tests thread/wait_group/storage.cpp)

# Sched / ThreadPool
add_concur_test(sched_thread_pool_unit_tests sched/thread_pool/unit.cpp)
add_concur_test(sched_thread_pool_random_tests sched/thread_pool/random.cpp)
add_concur_test(sched_thread_pool_stress_tests sched/thread_pool/stress.cpp)

add_concur_test(sched_thread_pool_load_balancing_random_tests sched/thread_pool/load_balancing/random.cpp)
add_concur_test(sched_thread_pool_load_balancing_random2_tests sched/thread_pool/load_balancing/random2.cpp)

# Sched / RunLoop
add_concur_test(sched_run_loop_unit_tests sched/run_loop/unit.cpp)
add_concur_test(sched_run_loop_intrusive_tests sched/run_loop/intrusive.cpp)
add_concur_test(sched_run_loop_sane_tests sched/run_loop/sane.cpp)
add_concur_test(sched_run_loop_alloc_tests sched/run_loop/alloc.cpp)

# Fibers

# Fiber / Sched
add_concur_test(fiber_sched_thread_pool_unit_tests fiber/sched/thread_pool/unit.cpp)
add_concur_test(fiber_sched_run_loop_unit_tests fiber/sched/run_loop/unit.cpp)
add_concur_test(fiber_sched_run_loop_sync_tests fiber/sched/run_loop/sync.cpp)

# Fiber / Sync

# Fiber / Sync / Event

add_concur_test(fiber_sync_event_unit_tests fiber/sync/event/unit.cpp)
add_concur_test(fiber_sync_event_random_tests fiber/sync/event/random.cpp)
add_concur_test(fiber_sync_event_stress_tests fiber/sync/event/stress.cpp)

add_concur_test(fiber_sync_event_storage_random_tests fiber/sync/event/storage/random.cpp)
add_concur_test(fiber_sync_event_storage_stress_tests fiber/sync/event/storage/stress.cpp)

# Fiber / Sync / Mutex

add_concur_test(fiber_sync_mutex_unit_tests fiber/sync/mutex/unit.cpp)
add_concur_test(fiber_sync_mutex_unit_tp_tests fiber/sync/mutex/unit_tp.cpp)
add_concur_test(fiber_sync_mutex_random_tests fiber/sync/mutex/random.cpp)
add_concur_test(fiber_sync_mutex_stress_tests fiber/sync/mutex/stress.cpp)
add_concur_test(fiber_sync_mutex_alloc_tests fiber/sync/mutex/alloc.cpp)

# Fiber / Sync / Strand

add_concur_test(fiber_sync_strand_unit_tests fiber/sync/strand/unit.cpp)
add_concur_test(fiber_sync_strand_random_tests fiber/sync/strand/random.cpp)
add_concur_test(fiber_sync_strand_stress_tests fiber/sync/strand/stress.cpp)
add_concur_test(fiber_sync_strand_switch_tests fiber/sync/strand/switch.cpp)
add_concur_test(fiber_sync_strand_alloc_tests fiber/sync/strand/alloc.cpp)

# Fiber / Sync / WaitGroup

add_concur_test(fiber_sync_wait_group_unit_tests fiber/sync/wait_group/unit.cpp)
add_concur_test(fiber_sync_wait_group_random_tests fiber/sync/wait_group/random.cpp)
add_concur_test(fiber_sync_wait_group_stress_tests fiber/sync/wait_group/stress.cpp)

add_concur_test(fiber_sync_wait_group_storage_random_tests fiber/sync/wait_group/storage/random.cpp)
add_concur_test(fiber_sync_wait_group_storage_stress_tests fiber/sync/wait_group/storage/stress.cpp)

# Future
add_concur_test(future_unit_tests future/unit.cpp)
add_concur_test(future_lazy_unit_tests future/lazy_unit.cpp)
