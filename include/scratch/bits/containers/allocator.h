#pragma once

#include "scratch/bits/type-traits/integral-constant.h"

#include <cstddef>
#include <new>

namespace scratch {

template<class T>
struct allocator {
    using value_type = T;
    using propagate_on_container_move_assignment = true_type;
    using is_always_equal = true_type;

    allocator() = default;
    allocator(const allocator&) = default;

    template<class U> explicit allocator(const allocator<U>&) noexcept {}

    T *allocate(size_t n) { return static_cast<T *>(::operator new(n)); }
    void deallocate(T *p, size_t) { ::operator delete((void *)p); }
};

template<class A, class B> bool operator==(const allocator<A>&, const allocator<B>&) noexcept { return true; }
template<class A, class B> bool operator!=(const allocator<A>&, const allocator<B>&) noexcept { return false; }

} // namespace scratch
