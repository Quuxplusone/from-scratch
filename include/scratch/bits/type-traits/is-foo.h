#pragma once

#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-class.h"
#include "scratch/bits/type-traits/is-noncv-foo.h"
#include "scratch/bits/type-traits/remove-foo.h"

#include <cstddef>

namespace scratch {

template<typename T> struct is_array                 : false_type {};
template<typename T> struct is_array<T[]>            : true_type {};
template<typename T, size_t N> struct is_array<T[N]> : true_type {};

template<typename T> struct is_const                 : false_type {};
template<typename T> struct is_const<const T>        : true_type {};

template<class T> struct is_floating_point           : is_noncv_floating_point<remove_cv_t<T>> {};

template<class T> struct is_integral                 : is_noncv_integral<remove_cv_t<T>> {};

template<class T> struct is_member_pointer           : is_noncv_member_pointer<remove_cv_t<T>> {};

template<class T> struct is_null_pointer             : is_noncv_null_pointer<remove_cv_t<T>> {};

template<class T> struct is_pointer                  : is_noncv_pointer<remove_cv_t<T>> {};

template<typename T> struct is_reference             : false_type {};
template<typename T> struct is_reference<T&>         : true_type {};
template<typename T> struct is_reference<T&&>        : true_type {};

template<typename T, typename U> struct is_same      : false_type {};
template<typename T> struct is_same<T, T>            : true_type {};

template<typename T> struct is_void                  : is_noncv_void<remove_cv_t<T>> {};

template<typename T> struct is_volatile              : false_type {};
template<typename T> struct is_volatile<volatile T>  : true_type {};

template<typename T> inline constexpr bool is_array_v = is_array<T>::value;
template<typename T> inline constexpr bool is_const_v = is_const<T>::value;
template<typename T> inline constexpr bool is_floating_point_v = is_floating_point<T>::value;
template<typename T> inline constexpr bool is_integral_v = is_integral<T>::value;
template<typename T> inline constexpr bool is_member_pointer_v = is_member_pointer<T>::value;
template<typename T> inline constexpr bool is_null_pointer_v = is_null_pointer<T>::value;
template<typename T> inline constexpr bool is_pointer_v = is_pointer<T>::value;
template<typename T> inline constexpr bool is_reference_v = is_reference<T>::value;
template<typename T, typename U> inline constexpr bool is_same_v = is_same<T, U>::value;
template<typename T> inline constexpr bool is_void_v = is_void<T>::value;
template<typename T> inline constexpr bool is_volatile_v = is_volatile<T>::value;


// Here follow some traits that are still pretty simple,
// but which depend on the REALLY simple traits above.
// Notice that every type is either "compound" or "fundamental".

template<typename T> inline constexpr bool is_arithmetic_v = is_integral_v<T> || is_floating_point_v<T>;
template<typename T> inline constexpr bool is_compound_v = !(is_arithmetic_v<T> || is_null_pointer_v<T> || is_void_v<T>);
template<typename T> inline constexpr bool is_fundamental_v = is_arithmetic_v<T> || is_null_pointer_v<T> || is_void_v<T>;
template<typename T> inline constexpr bool is_scalar_v = is_arithmetic_v<T> || is_enum_v<T> || is_member_pointer_v<T> || is_null_pointer_v<T> || is_pointer_v<T>;
template<typename T> inline constexpr bool is_object_v = is_array_v<T> || is_class_v<T> || is_scalar_v<T> || is_union_v<T>;

template<typename T> struct is_arithmetic : bool_constant<is_arithmetic_v<T>> {};
template<typename T> struct is_compound : bool_constant<is_compound_v<T>> {};
template<typename T> struct is_fundamental : bool_constant<is_fundamental_v<T>> {};
template<typename T> struct is_object : bool_constant<is_object_v<T>> {};
template<typename T> struct is_scalar : bool_constant<is_scalar_v<T>> {};

} // namespace scratch
