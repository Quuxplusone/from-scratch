#pragma once

#include "scratch/bits/type-traits/integral-constant.h"

#include <cstddef>

namespace scratch {

template<typename T>                     struct rank            : index_constant<0> {};
template<typename T>                     struct rank<T[]>       : index_constant<rank<T>::value + 1> {};
template<typename T, size_t N>           struct rank<T[N]>      : index_constant<rank<T>::value + 1> {};

template<typename T, size_t = 0>         struct extent          : index_constant<0> {};
template<typename T>                     struct extent<T[], 0>  : index_constant<0> {};
template<typename T, size_t N>           struct extent<T[N], 0> : index_constant<N> {};
template<typename T, size_t I>           struct extent<T[], I>  : extent<T, I-1> {};
template<typename T, size_t N, size_t I> struct extent<T[N], I> : extent<T, I-1> {};

template<typename T>            struct remove_extent            { using type = T; };
template<typename T>            struct remove_extent<T[]>       { using type = T; };
template<typename T, size_t N>  struct remove_extent<T[N]>      { using type = T; };

template<typename T>            struct remove_all_extents       { using type = T; };
template<typename T>            struct remove_all_extents<T[]>  : remove_all_extents<T> {};
template<typename T, size_t N>  struct remove_all_extents<T[N]> : remove_all_extents<T> {};

template<typename T> inline constexpr size_t rank_v = rank<T>::value;
template<typename T, size_t I = 0> inline constexpr size_t extent_v = extent<T, I>::value;
template<typename T> using remove_extent_t = typename remove_extent<T>::type;
template<typename T> using remove_all_extents_t = typename remove_all_extents<T>::type;

} // namespace scratch
