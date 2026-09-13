#pragma once

#include<cstddef>
#include<cstdint>
#include<new>
#include<utility>
#include<algorithm>

namespace hft {

template<typename T>
class MemoryPool {
    private:
    struct Block {
        Block* next;
    };

    static constexpr std::size_t BlockSize = std::max(sizeof(T), sizeof(Block));
    static constexpr std::size_t BlockAlign = std::max(alignof(T), alignof(Block));

    Block* free_list_head_ = nullptr;

    std::byte* buffer_ = nullptr;

    std::size_t capacity_ = 0;

public:
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator = (const MemoryPool&) = delete;

    MemoryPool() = default;
};
    
}
