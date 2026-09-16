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

        // Constructs an item directly in the ring buffer slot
    template <typename... Args>
    [[nodiscard]] bool emplace(Args&&... args) {
        const std::size_t current_head = head_.load(std::memory_order_relaxed);
        const std::size_t current_tail = tail_.load(std::memory_order_acquire);

        // Check if queue is full
        if (current_head - current_tail >= Capacity) {
            return false; // Queue is full, cannot push
        }

        // 1. Calculate slot index using fast bitwise mask
        auto* slot = reinterpret_cast<T*>(buffer_[current_head & BufferMask].storage);

        // 2. Placement new: construct object in-place
        new (slot) T(std::forward<Args>(args)...);

        // 3. Publish the new head with release semantics
        head_.store(current_head + 1, std::memory_order_release);

        return true;
    }

    // Pushes an item by copying or moving
    [[nodiscard]] bool push(const T& item) {
        return emplace(item);
    }

    [[nodiscard]] bool push(T&& item) {
        return emplace(std::move(item));
    }

        // Pops an item from the queue; returns false if the queue is empty
    [[nodiscard]] bool pop(T& value) {
        const std::size_t current_tail = tail_.load(std::memory_order_relaxed);
        const std::size_t current_head = head_.load(std::memory_order_acquire);

        // Check if queue is empty
        if (current_head == current_tail) {
            return false; // Queue is empty
        }

        // 1. Locate slot using bitwise mask
        auto* slot = reinterpret_cast<T*>(buffer_[current_tail & BufferMask].storage);

        // 2. Move the object out to caller
        value = std::move(*slot);

        // 3. Destroy the object in the slot
        slot->~T();

        // 4. Publish updated tail to free the slot for the Producer
        tail_.store(current_tail + 1, std::memory_order_release);

        return true;
    }

    // Returns true if the queue is currently empty
    [[nodiscard]] bool empty() const noexcept {
        return head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed);
    }

    // Returns the approximate number of items in the queue
    [[nodiscard]] std::size_t size() const noexcept {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        return (head >= tail) ? (head - tail) : 0;
    }

    // Returns the maximum capacity of the queue
    [[nodiscard]] constexpr std::size_t capacity() const noexcept {
        return Capacity;
    }


};

} // namespace hft
