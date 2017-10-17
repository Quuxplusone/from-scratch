#pragma once

#include "scratch/bits/type-traits/integral-constant.h"

namespace scratch {

template<class T>
struct inplace_allocator {
    using value_type = T;
    constexpr inplace_allocator() noexcept = default;
    template<class U> constexpr inplace_allocator(const inplace_allocator<U>&) noexcept {}
};

template<class T> struct is_inplace_allocator : false_type {};
template<class T> struct is_inplace_allocator<inplace_allocator<T>> : true_type {};
template<class T> inline constexpr bool is_inplace_allocator_v = is_inplace_allocator<T>::value;

} // namespace scratch
