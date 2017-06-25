#pragma once

#include "scratch/bits/aligned-storage/max-align-t.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/enable-if.h"

#include <cstddef>

namespace scratch::detail {

template<size_t, class = void> struct default_alignment : index_constant<alignof(max_align_t)> {};
template<> struct default_alignment<1, void> : index_constant<1> {};
template<size_t Len> struct default_alignment<Len, enable_if_t<(2 <= Len && Len <= 3)>> : index_constant<2> {};
template<size_t Len> struct default_alignment<Len, enable_if_t<(4 <= Len && Len <= 7)>> : index_constant<4> {};
template<size_t Len> struct default_alignment<Len, enable_if_t<(8 <= Len && Len <= 15)>> : index_constant<8> {};

template<size_t Len> inline constexpr size_t default_alignment_v = default_alignment<Len>::value;

} // namespace scratch::detail

namespace scratch {

template<size_t Len, size_t Align = detail::default_alignment_v<Len>>
struct aligned_storage {
    using type = struct {
        alignas(Align) char data[Len];
    };

    virtual void you_probably_meant_to_use_the_member_typedef_type() = 0;
};

template<size_t Len, size_t Align = detail::default_alignment_v<Len>>
using aligned_storage_t = typename aligned_storage<Len, Align>::type;

} // namespace scratch
