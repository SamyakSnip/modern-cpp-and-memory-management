#include <iostream>
#include <cassert>
#include <cstdint>
#include "hft_mem/memory_pool.hpp"

struct Order {
    uint64_t order_id;
    double price;
    uint32_t quantity;

    Order(uint64_t id, double p, uint32_t q)
        : order_id(id), price(p), quantity(q) {}
};

void test_raw_allocation_and_exhaustion() {
    std::cout << "[1] Testing Raw Allocation, Alignment & Exhaustion...\n";
    constexpr std::size_t Capacity = 3;
    hft::MemoryPool<Order> pool(Capacity);

    Order* o1 = pool.allocate();
    assert(o1 != nullptr);
    // Verify hardware alignment
    assert(reinterpret_cast<std::uintptr_t>(o1) % alignof(Order) == 0);

    Order* o2 = pool.allocate();
    Order* o3 = pool.allocate();
    assert(o2 != nullptr && o3 != nullptr);

    // Pool exhaustion test: 4th allocation must safely return nullptr
    Order* o4 = pool.allocate();
    assert(o4 == nullptr);

    pool.deallocate(o1);
    pool.deallocate(o2);
    pool.deallocate(o3);
    std::cout << "    PASSED: Raw allocation, alignment, and exhaustion.\n";
}

void test_create_and_destroy() {
    std::cout << "[2] Testing Object Construction, Destruction & LIFO Reuse...\n";
    constexpr std::size_t Capacity = 3;
    hft::MemoryPool<Order> pool(Capacity);

    // 1. Placement new via create()
    Order* o1 = pool.create(101, 450.5, 100);
    assert(o1 != nullptr);
    assert(o1->order_id == 101);
    assert(o1->price == 450.5);
    assert(o1->quantity == 100);

    // 2. Destructor invocation via destroy()
    pool.destroy(o1);

    // 3. Verify LIFO memory reuse
    Order* o2 = pool.create(102, 999.0, 50);
    assert(o2 == o1); // Reuses the exact same memory address!
    assert(o2->order_id == 102);

    pool.destroy(o2);
    std::cout << "    PASSED: Object lifecycle and free-list recycling.\n";
}

int main() {
    std::cout << "=== Running HFT Memory Pool Test Suite ===\n";
    test_raw_allocation_and_exhaustion();
    test_create_and_destroy();
    std::cout << "=== All Memory Pool Tests Passed Successfully! ===\n";
    return 0;
}
