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

add_concur_test(fiber_sync_buffered_channel_unit_tests fiber/sync/channel/buffered/unit.cpp)
add_concur_test(fiber_sync_buffered_channel_random_tests fiber/sync/channel/buffered/random.cpp)
add_concur_test(fiber_sync_buffered_channel_stress_tests fiber/sync/channel/buffered/stress.cpp)

# Future
add_concur_test(future_unit_tests future/unit.cpp)
add_concur_test(future_lazy_unit_tests future/lazy_unit.cpp)

# Stackless Coroutines

# Coro / Sched
add_concur_test(coro_sched_run_loop_unit_tests coro/sched/run_loop/unit.cpp)
add_concur_test(coro_sched_thread_pool_unit_tests coro/sched/thread_pool/unit.cpp)

# Coro / Sync / Event
add_concur_test(coro_sync_event_unit_tests coro/sync/event/unit.cpp)
add_concur_test(coro_sync_event_random_tests coro/sync/event/random.cpp)
add_concur_test(coro_sync_event_stress_tests coro/sync/event/stress.cpp)

add_concur_test(coro_sync_event_storage_random_tests coro/sync/event/storage/random.cpp)
add_concur_test(coro_sync_event_storage_stress_tests coro/sync/event/storage/stress.cpp)

# Coro / Sync / Mutex / ScopedLock
add_concur_test(coro_sync_mutex_scoped_lock_unit_tests coro/sync/mutex/scoped_lock/unit.cpp)
add_concur_test(coro_sync_mutex_scoped_lock_random_tests coro/sync/mutex/scoped_lock/random.cpp)
add_concur_test(coro_sync_mutex_scoped_lock_stress_tests coro/sync/mutex/scoped_lock/stress.cpp)

# Coro / Sync / Mutex / RunMutex
add_concur_test(coro_sync_mutex_run_unit_tests coro/sync/mutex/run/unit.cpp)
add_concur_test(coro_sync_mutex_run_random_tests coro/sync/mutex/run/random.cpp)
add_concur_test(coro_sync_mutex_run_stress_tests coro/sync/mutex/run/stress.cpp)

# Coro / Sync / WaitGroup
add_concur_test(coro_sync_wait_group_unit_tests coro/sync/wait_group/unit.cpp)
add_concur_test(coro_sync_wait_group_random_tests coro/sync/wait_group/random.cpp)
add_concur_test(coro_sync_wait_group_stress_tests coro/sync/wait_group/stress.cpp)

add_concur_test(coro_sync_wait_group_storage_random_tests coro/sync/wait_group/storage/random.cpp)
add_concur_test(coro_sync_wait_group_storage_stress_tests coro/sync/wait_group/storage/stress.cpp)

# Lock-free 

# Lock-free / Stack
add_concur_test(lf_stack_unit_tests lf/stack/unit.cpp)
add_concur_test(lf_stack_random_tests lf/stack/random.cpp)
add_concur_test(lf_stack_stress_tests lf/stack/stress.cpp)

# Lock-free / Queue
add_concur_test(lf_queue_unit_tests lf/queue/unit.cpp)
add_concur_test(lf_queue_random_tests lf/queue/random.cpp)
add_concur_test(lf_queue_stress_lf_tests lf/queue/stress/lf.cpp)
add_concur_test(lf_queue_stress_lf_mm_tests lf/queue/stress/lf_mm.cpp)
