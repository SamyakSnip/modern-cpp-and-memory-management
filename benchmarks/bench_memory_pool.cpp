#include <iostream>
#include <chrono>
#include <vector>
#include <cstdint>
#include <iomanip>
#include "hft_mem/memory_pool.hpp"

// Sample order payload
struct Order {
    uint64_t order_id;
    double price;
    uint32_t quantity;
    char symbol[8];
};

// Prevents compiler from optimizing away unused variables (Dead Code Elimination)
template <typename T>
inline void do_not_optimize(T* ptr) {
    // A volatile read tells the compiler: "This memory has side-effects, do not delete it!"
    reinterpret_cast<volatile char*>(ptr)[0] = 0;
}

int main() {
    constexpr std::size_t Iterations = 1'000'000;
    std::cout << "========================================================\n";
    std::cout << "  HFT MEMORY POOL vs STANDARD HEAP BENCHMARK (" << Iterations << " ops)\n";
    std::cout << "========================================================\n\n";

    // -------------------------------------------------------------
    // Benchmark 1: Standard OS Heap (new / delete)
    // -------------------------------------------------------------
    std::cout << "[1/2] Benchmarking Standard new / delete...\n";
    auto start_heap = std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < Iterations; ++i) {
        Order* o = new Order();
        do_not_optimize(o);
        delete o;
    }

    auto end_heap = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> heap_duration = end_heap - start_heap;
    double heap_ns_per_op = (heap_duration.count() * 1'000'000.0) / Iterations;

    std::cout << "      Total Time:  " << std::fixed << std::setprecision(2) << heap_duration.count() << " ms\n";
    std::cout << "      Avg Latency: " << std::fixed << std::setprecision(2) << heap_ns_per_op << " ns / op\n\n";

    // -------------------------------------------------------------
    // Benchmark 2: HFT Fixed-Size Block Memory Pool
    // -------------------------------------------------------------
    std::cout << "[2/2] Benchmarking hft::MemoryPool...\n";
    hft::MemoryPool<Order> pool(1024); // Pre-allocated pool of 1024 slots

    auto start_pool = std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < Iterations; ++i) {
        Order* o = pool.allocate();
        do_not_optimize(o);
        pool.deallocate(o);
    }

    auto end_pool = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> pool_duration = end_pool - start_pool;
    double pool_ns_per_op = (pool_duration.count() * 1'000'000.0) / Iterations;

    std::cout << "      Total Time:  " << std::fixed << std::setprecision(2) << pool_duration.count() << " ms\n";
    std::cout << "      Avg Latency: " << std::fixed << std::setprecision(2) << pool_ns_per_op << " ns / op\n\n";

    // -------------------------------------------------------------
    // Speedup Comparison
    // -------------------------------------------------------------
    double speedup = heap_duration.count() / pool_duration.count();
    std::cout << "========================================================\n";
    std::cout << "  RESULT: hft::MemoryPool is " 
              << std::fixed << std::setprecision(2) << speedup 
              << "x FASTER than standard new/delete!\n";
    std::cout << "========================================================\n";

    return 0;
}
