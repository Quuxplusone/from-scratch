#pragma once

#include "scratch/bits/type-traits/compiler-magic.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/void-t.h"

#include <cstddef>
#include <utility>

namespace scratch {

template<class T, size_t N = 100, bool OmitArray = is_abstract_v<T> || is_void_v<T>>
struct constexpr_allocator {
    using value_type = T;

    constexpr constexpr_allocator() = default;
    template<class U, size_t N2> constexpr constexpr_allocator(const constexpr_allocator<U, N2>&) {}
    template<class U> using rebind = constexpr_allocator<U, N>;

    constexpr T *allocate(size_t);

    template<class U, class... Args>
    constexpr void construct(U *p, Args&&... args) {
        *p = T(std::forward<Args>(args)...);
    }

    template<class U>
    constexpr void destroy(U *) {}
};

template<class T, size_t N>
struct constexpr_allocator<T, N, false> {
    T m_data[N];
    size_t m_used = 0;
    using value_type = T;

    constexpr constexpr_allocator() = default;
    template<class U, size_t N2> constexpr constexpr_allocator(const constexpr_allocator<U, N2>&) {}
    template<class U> using rebind = constexpr_allocator<U, N>;

    constexpr T *allocate(size_t n) {
        m_used += n;
        return &m_data[m_used - n];
    }

    template<class U, class... Args>
    constexpr void construct(U *p, Args&&... args) {
        *p = T(std::forward<Args>(args)...);
    }

    template<class U>
    constexpr void destroy(U *) {}
};

template<class T> struct is_constexpr_allocator : false_type {};
template<class T, size_t N> struct is_constexpr_allocator<constexpr_allocator<T, N>> : true_type {};
template<class T> inline constexpr bool is_constexpr_allocator_v = is_constexpr_allocator<T>::value;

} // namespace scratch
