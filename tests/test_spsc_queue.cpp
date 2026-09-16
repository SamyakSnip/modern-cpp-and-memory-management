#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <cstdint>
#include "hft_mem/spsc_queue.hpp"

void test_basic_fifo() {
    std::cout << "[1] Testing Basic FIFO and Queue Limits...\n";
    // Capacity must be power of two (8 slots)
    hft::SpscQueue<int, 8> queue;

    assert(queue.empty());
    assert(queue.size() == 0);

    // Fill the queue
    for (int i = 0; i < 8; ++i) {
        bool pushed = queue.push(i * 10);
        assert(pushed);
    }

    // 9th push must fail (Queue is full)
    bool pushed_overflow = queue.push(999);
    assert(!pushed_overflow);
    assert(queue.size() == 8);

    // Pop and verify strict FIFO ordering
    for (int i = 0; i < 8; ++i) {
        int val = -1;
        bool popped = queue.pop(val);
        assert(popped);
        assert(val == i * 10);
    }

    // Queue should now be empty
    int dummy = 0;
    assert(!queue.pop(dummy));
    assert(queue.empty());
    std::cout << "    PASSED: Basic FIFO and capacity limit checks.\n";
}

void test_multithreaded_stress() {
    std::cout << "[2] Testing Lock-Free Multithreaded Stress (1,000,000 items)...\n";
    constexpr std::size_t QueueCapacity = 1024; // Power of two
    constexpr std::size_t TotalItems = 1'000'000;

    hft::SpscQueue<std::uint64_t, QueueCapacity> queue;

    // Producer Thread: Pumps 1,000,000 integers
    std::thread producer([&queue]() {
        for (std::uint64_t i = 0; i < TotalItems; ++i) {
            // Keep trying until space is available in the ring buffer
            while (!queue.push(i)) {
                std::this_thread::yield(); // Cooperatively yield CPU core to consumer
            }
        }
    });

    // Consumer Thread: Pops 1,000,000 integers and verifies strict sequential order
    std::thread consumer([&queue]() {
        for (std::uint64_t expected = 0; expected < TotalItems; ++expected) {
            std::uint64_t received = 0;
            while (!queue.pop(received)) {
                std::this_thread::yield();
            }
            // Strict FIFO correctness assertion
            assert(received == expected);
        }
    });

    producer.join();
    consumer.join();

    assert(queue.empty());
    std::cout << "    PASSED: 1,000,000 items transferred with 0 drops and 0 corruptions!\n";
}

int main() {
    std::cout << "=== Running HFT SPSC Queue Test Suite ===\n";
    test_basic_fifo();
    test_multithreaded_stress();
    std::cout << "=== All SPSC Queue Tests Passed Successfully! ===\n";
    return 0;
}
