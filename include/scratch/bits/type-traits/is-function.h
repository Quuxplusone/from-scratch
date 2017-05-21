#pragma once

#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"

namespace scratch {

// is_normal_function is NOT a standard library type trait!
template<class T> struct is_non_abominable_function : false_type {};
template<class R, class... A> struct is_non_abominable_function<R(A...)> : true_type {};
template<class R, class... A> struct is_non_abominable_function<R(A..., ...)> : true_type {};
template<class T> inline constexpr bool is_non_abominable_function_v = is_non_abominable_function<T>::value;

template<typename T> inline constexpr bool is_function_v = !is_const_v<const T> && !is_reference_v<T>;
template<typename T> struct is_function : bool_constant<is_function_v<T>> {};

// is_abominable_function is NOT a standard library type trait!
template<class T> inline constexpr bool is_abominable_function_v = is_function_v<T> && !is_non_abominable_function_v<T>;
template<class T> struct is_abominable_function : bool_constant<is_abominable_function_v<T>> {};

template<typename T> inline constexpr bool is_member_function_pointer_v = is_member_pointer_v<T> && is_function_v<remove_pointer_t<T>>;
template<typename T> struct is_member_function_pointer : bool_constant<is_member_function_pointer_v<T>> {};

template<typename T> inline constexpr bool is_member_object_pointer_v = is_member_pointer_v<T> && !is_function_v<remove_pointer_t<T>>;
template<typename T> struct is_member_object_pointer : bool_constant<is_member_object_pointer_v<T>> {};

// is_referenceable is NOT a standard type trait!
template<typename T> inline constexpr bool is_referenceable_v = is_object_v<T> || is_non_abominable_function_v<T> || is_reference_v<T>;
template<typename T> struct is_referenceable : bool_constant<is_referenceable_v<T>> {};

} // namespace scratch
