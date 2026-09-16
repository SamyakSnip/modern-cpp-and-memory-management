#pragma once

#include <cstddef>
#include <new>
#include "hft_mem/memory_pool.hpp"

namespace hft {

template <typename T>
class PoolAllocator {
public:
    using value_type = T;

private:
    MemoryPool<T>* pool_ = nullptr;

public:
    PoolAllocator() noexcept = default;

    explicit PoolAllocator(MemoryPool<T>& pool) noexcept : pool_(&pool) {}

    template <typename U>
    PoolAllocator(const PoolAllocator<U>& other) noexcept : pool_(reinterpret_cast<MemoryPool<T>*>(other.get_pool())) {}

    [[nodiscard]] void* get_pool() const noexcept {
        return pool_;
    }

    [[nodiscard]] T* allocate(std::size_t n) {
        if (n == 0) return nullptr;
        if(n>1 || pool_ == nullptr) {
        throw std::bad_alloc();
        }
        T* ptr = pool_->allocate();
        if (ptr == nullptr){
            throw std::bad_alloc();
        }
        return ptr;
    }

    void deallocate(T* ptr, std::size_t /*n*/) noexcept {
        if(pool_ != nullptr && ptr != nullptr) {
            pool_->deallocate(ptr);
        }
    }

    template <typename U>
    bool operator == (const PoolAllocator<U>& other) const noexcept {
        return pool_ == other.get_pool();
    }
};
    

}