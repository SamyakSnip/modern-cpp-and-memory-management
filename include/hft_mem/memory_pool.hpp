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

    static constexpr std::size_t BlockAlign = std::max(alignof(T), alignof(Block));
    static constexpr std::size_t RawBlockSize = std::max(sizeof(T), sizeof(Block));
    static constexpr std::size_t BlockSize = ((RawBlockSize + BlockAlign - 1) / BlockAlign) * BlockAlign;


    Block* free_list_head_ = nullptr;

    std::byte* buffer_ = nullptr;

    std::size_t capacity_ = 0;

public:
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator = (const MemoryPool&) = delete;

    MemoryPool() = default;

    // constructor : pre_allocates all memory on the heap and initializes free list
    explicit MemoryPool(std::size_t capacity):capacity_(capacity) 
    {
        if(capacity == 0) return;

        std::size_t total_bytes = capacity_ * BlockSize;
        buffer_ = static_cast<std::byte*>(
            ::operator new(total_bytes, std::align_val_t{BlockAlign})
        );

        for(std::size_t i = 0; i < capacity_; i++){

            Block* current = reinterpret_cast<Block*>(buffer_ + (i* BlockSize));

        if(i+1 < capacity_){

            Block* next_block = reinterpret_cast<Block*>(buffer_+((i+1) * BlockSize));
            current->next = next_block;

        } else {
            current->next = nullptr;
        }
        }

        free_list_head_ = reinterpret_cast<Block*>(buffer_);
    }

    // distuctor: frees the entire buffer in one go
    ~MemoryPool() {
        if(buffer_ != nullptr) {
            ::operator delete(buffer_, std::align_val_t{BlockAlign});
            buffer_ = nullptr;
            free_list_head_ = nullptr;
            capacity_ = 0;
        }
    }

    // O(1) Hot-Path Allocation: Pop head from free list
    [[nodiscard]] T* allocate() {
        if (free_list_head_ == nullptr) {
            // Pool is exhausted (Out of memory)
            return nullptr;
        }

        Block* block = free_list_head_;
        free_list_head_ = free_list_head_->next;
        return reinterpret_cast<T*>(block);
    }

    // O(1) Hot-Path Deallocation: Push block back to head of free list
    void deallocate(T* ptr) noexcept {
        if (ptr == nullptr) return;

        Block* block = reinterpret_cast<Block*>(ptr);
        block->next = free_list_head_;
        free_list_head_ = block;
    }



};
    
}
