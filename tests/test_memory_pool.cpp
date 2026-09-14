#include<iostream>
#include<cassert>
#include<cstdint>
#include"hft_mem/memory_pool.hpp"

struct Order {
    uint64_t order_id;
    double price;
    uint32_t quantity;
};

int main() {
    std::cout << "=== Running HFT Memory Pool Test === \n";

    //1. create pool for 3 orders
    constexpr std::size_t PoolCapacity = 3;
    hft::MemoryPool<Order> pool(PoolCapacity);

    Order* o1 = pool.allocate();
    assert(o1 != nullptr);
    std::cout<<"Allocated o1 at address: " << o1 << "\n";
    
    assert(reinterpret_cast<std::uintptr_t>(o1)% alignof(Order) == 0);
    
    Order* o2 = pool.allocate();
    Order* o3 = pool.allocate();
    std::cout<<"Allocated o2 at address: " << o2 << "\n";
    std::cout<<"Allocated o3 at address: " << o3 << "\n";
    
    //4. Test Pool Exhaustion : 4th allocation must fail
    Order* o4 = pool.allocate();
    assert(o4 == nullptr);
    std::cout<<"Pool correctly returned nullptr when capacity was exhausted.\n";

    std::cout << "Deallocating o2(" << o2 << ")...\n";
    pool.deallocate(o2);

    Order* reused = pool.allocate();
    std::cout << "Reallocated block at address: " << reused << "\n";
    assert(reused == o2);
    std::cout << "SUCCESS: Free list reused the exact same memory address!\n";
    // Clean up
    pool.deallocate(o1);
    pool.deallocate(o3);
    pool.deallocate(reused);
    std::cout << "=== All Memory Pool Tests Passed! ===\n";
    return 0;
}



