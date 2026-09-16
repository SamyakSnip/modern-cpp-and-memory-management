#pragma once

#include <cstddef>
#include <atomic>
#include <new>
#include <utility>
#include <bit>

namespace hft {

template <typename T, std::size_t Capacity>
class SpscQueue {
    // 1. Enforce Power-of-Two Capacity at Compile-Time
    static_assert(Capacity >= 2, "Capacity must be at least 2");
    static_assert(std::has_single_bit(Capacity), "Capacity must be a power of two!");

    // Hardware cache line size on modern x86/ARM CPUs
    static constexpr std::size_t CacheLineSize = 64;

    // Bitmask for fast single-cycle modulo wrapping: index & BufferMask
    static constexpr std::size_t BufferMask = Capacity - 1;

    // 2. Raw uninitialized storage for objects of type T
    // Using alignas(alignof(T)) guarantees proper hardware alignment
    struct alignas(alignof(T)) StorageSlot {
        std::byte storage[sizeof(T)];
    };
    StorageSlot buffer_[Capacity];

    // 3. Producer state (Modified ONLY by Producer Thread)
    // alignas(64) puts head on its own dedicated cache line to prevent False Sharing!
    alignas(CacheLineSize) std::atomic<std::size_t> head_{0};

    // 4. Consumer state (Modified ONLY by Consumer Thread)
    // alignas(64) puts tail on its own dedicated cache line!
    alignas(CacheLineSize) std::atomic<std::size_t> tail_{0};

public:
    SpscQueue() = default;

    // No copying allowed for a concurrent lock-free queue
    SpscQueue(const SpscQueue&) = delete;
    SpscQueue& operator=(const SpscQueue&) = delete;

    ~SpscQueue() {
        // Drain and destroy any remaining active objects
        T dummy;
        while (pop(dummy)) {}
    }
};

} // namespace hft
