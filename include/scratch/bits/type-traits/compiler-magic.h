#pragma once

#include "scratch/bits/type-traits/integral-constant.h"

namespace scratch {

template<class T> struct has_virtual_destructor : bool_constant<__has_virtual_destructor(T)> {};
template<class T> struct is_aggregate           : bool_constant<__is_aggregate(T)> {};
template<class T> struct is_empty               : bool_constant<__is_empty(T)> {};
template<class T> struct is_enum                : bool_constant<__is_enum(T)> {};
template<class T> struct is_final               : bool_constant<__is_final(T)> {};
template<class T> struct is_literal_type        : bool_constant<__is_literal_type(T)> {};
template<class T> struct is_pod                 : bool_constant<__is_pod(T)> {};
template<class T> struct is_standard_layout     : bool_constant<__is_standard_layout(T)> {};
template<class T> struct is_trivially_copyable  : bool_constant<__is_trivially_copyable(T)> {};
template<class T> struct is_union               : bool_constant<__is_union(T)> {};

template<class T> inline constexpr bool has_virtual_destructor_v = __has_virtual_destructor(T);
template<class T> inline constexpr bool is_aggregate_v           = __is_aggregate(T);
template<class T> inline constexpr bool is_empty_v               = __is_empty(T);
template<class T> inline constexpr bool is_enum_v                = __is_enum(T);
template<class T> inline constexpr bool is_final_v               = __is_final(T);
template<class T> inline constexpr bool is_literal_type_v        = __is_literal_type(T);
template<class T> inline constexpr bool is_pod_v                 = __is_pod(T);
template<class T> inline constexpr bool is_standard_layout_v     = __is_standard_layout(T);
template<class T> inline constexpr bool is_trivially_copyable_v  = __is_trivially_copyable(T);
template<class T> inline constexpr bool is_union_v               = __is_union(T);

template<class T, class... Us> struct is_trivially_constructible : bool_constant<__is_trivially_constructible(T, Us...)> {};
template<class T, class U>     struct is_trivially_assignable    : bool_constant<__is_trivially_assignable(T, U)> {};

template<class T, class... Us> inline constexpr bool is_trivially_constructible_v = __is_trivially_constructible(T, Us...);
template<class T, class U>     inline constexpr bool is_trivially_assignable_v = __is_trivially_assignable(T, U);

template<class T> struct underlying_type { using type = __underlying_type(T); };

template<class T> using underlying_type_t = typename underlying_type<T>::type;

} // namespace scratch
