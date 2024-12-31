#pragma once

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <new>
namespace neuro {
#include <malloc.h>
#include <stdlib.h>

template <typename T> struct AlignmentAllocator {
    using value_type = T;

    AlignmentAllocator() = default;

    template <typename U> AlignmentAllocator(const AlignmentAllocator<U>&) {}

    T* allocate(std::size_t n) {
        void* ptr = nullptr;

        ptr = aligned_alloc(32, (n * sizeof(T) + 31) / 32 * 32);
        if (!ptr) {
            throw std::bad_alloc();
        }

        return static_cast<T*>(ptr);
    }

    void deallocate(T* ptr, std::size_t) {
        free(ptr); // Free memory
    }

    bool operator==(const AlignmentAllocator<T>&) { return true; }
    bool operator!=(const AlignmentAllocator<T>&) { return false; }
};
} // namespace neuro
