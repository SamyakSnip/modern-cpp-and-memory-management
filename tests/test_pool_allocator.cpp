#include <iostream>
#include <cassert>
#include <memory>
#include "hft_mem/memory_pool.hpp"
#include "hft_mem/pool_allocator.hpp"

struct MarketTick {
    uint64_t timestamp;
    double price;
    uint32_t volume;
};

void test_stl_traits_compliance() {
    std::cout << "[1] Testing std::allocator_traits compliance...\n";
    hft::MemoryPool<MarketTick> pool(2);
    hft::PoolAllocator<MarketTick> alloc(pool);

    // Use official std::allocator_traits interface
    MarketTick* tick1 = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
    assert(tick1 != nullptr);

    // Initialize fields
    tick1->timestamp = 1700000000;
    tick1->price = 150.25;
    tick1->volume = 500;
    assert(tick1->price == 150.25);

    std::allocator_traits<decltype(alloc)>::deallocate(alloc, tick1, 1);
    std::cout << "    PASSED: std::allocator_traits allocation and deallocation.\n";
}

void test_bad_alloc_exceptions() {
    std::cout << "[2] Testing std::bad_alloc exception guarantees...\n";
    hft::MemoryPool<MarketTick> pool(1);
    hft::PoolAllocator<MarketTick> alloc(pool);

    MarketTick* t1 = alloc.allocate(1);
    assert(t1 != nullptr);

    // 1. Capacity exhausted -> Must throw std::bad_alloc
    bool caught_exhaustion = false;
    try {
        alloc.allocate(1);
    } catch (const std::bad_alloc&) {
        caught_exhaustion = true;
    }
    assert(caught_exhaustion);

    // 2. Requesting n > 1 -> Must throw std::bad_alloc
    bool caught_multi_block = false;
    try {
        alloc.allocate(5);
    } catch (const std::bad_alloc&) {
        caught_multi_block = true;
    }
    assert(caught_multi_block);

    alloc.deallocate(t1, 1);
    std::cout << "    PASSED: Exception safety on exhaustion and multi-block request.\n";
}

void test_allocator_equality() {
    std::cout << "[3] Testing allocator equality comparison...\n";
    hft::MemoryPool<MarketTick> pool1(5);
    hft::MemoryPool<MarketTick> pool2(5);

    hft::PoolAllocator<MarketTick> alloc1(pool1);
    hft::PoolAllocator<MarketTick> alloc1_clone(pool1);
    hft::PoolAllocator<MarketTick> alloc2(pool2);

    assert(alloc1 == alloc1_clone); // Same pool -> Equal
    assert(alloc1 != alloc2);       // Different pools -> Not equal
    std::cout << "    PASSED: Allocator equality semantics.\n";
}

int main() {
    std::cout << "=== Running HFT Pool Allocator Test Suite ===\n";
    test_stl_traits_compliance();
    test_bad_alloc_exceptions();
    test_allocator_equality();
    std::cout << "=== All Pool Allocator Tests Passed Successfully! ===\n";
    return 0;
}
