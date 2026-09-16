add_executable(test_memory_pool test_memory_pool.cpp)
add_test(NAME MemoryPoolTest COMMAND test_memory_pool)

add_executable(test_pool_allocator test_pool_allocator.cpp)
add_test(NAME PoolAllocatorTest COMMAND test_pool_allocator)

add_executable(test_spsc_queue test_spsc_queue.cpp)
add_test(NAME SpscQueueTest COMMAND test_spsc_queue)
