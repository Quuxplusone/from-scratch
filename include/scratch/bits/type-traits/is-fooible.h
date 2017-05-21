#pragma once

#include "scratch/bits/type-traits/add-foo.h"
#include "scratch/bits/type-traits/compiler-magic.h"
#include "scratch/bits/type-traits/extent.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/utility/declval.h"

#include <new>

namespace scratch::detail {

template<class T, class Enable, class... Us> struct is_constructible_impl                                                                         : false_type {};
template<class T, class... Us>               struct is_constructible_impl<T, decltype(void(::new (declval<void*>()) T(declval<Us>()...))), Us...> : true_type {};

template<class T, class Enable, class... Us> struct is_nothrow_constructible_impl                                                                         : false_type {};
template<class T, class... Us>               struct is_nothrow_constructible_impl<T, decltype(void(::new (declval<void*>()) T(declval<Us>()...))), Us...> : bool_constant<noexcept(::new (declval<void*>()) T(declval<Us>()...))> {};

template<class T, class U, class Enable> struct is_assignable_impl                                                    : false_type {};
template<class T, class U>               struct is_assignable_impl<T, U, decltype(void(declval<T>() = declval<U>()))> : true_type {};

template<class T, class U, class Enable> struct is_nothrow_assignable_impl                                                    : false_type {};
template<class T, class U>               struct is_nothrow_assignable_impl<T, U, decltype(void(declval<T>() = declval<U>()))> : bool_constant<noexcept(declval<T>() = declval<U>())> {};

template<class T, class Enable> struct is_destructible_impl                                  : false_type {};
template<class T>               struct is_destructible_impl<T&, void>                        : true_type {};
template<class T>               struct is_destructible_impl<T&&, void>                       : true_type {};
template<class T>               struct is_destructible_impl<T, decltype(declval<T&>().~T())> : true_type {};

template<class T, class Enable> struct is_nothrow_destructible_impl                                  : false_type {};
template<class T>               struct is_nothrow_destructible_impl<T&, void>                        : true_type {};
template<class T>               struct is_nothrow_destructible_impl<T&&, void>                       : true_type {};
template<class T>               struct is_nothrow_destructible_impl<T, decltype(declval<T&>().~T())> : bool_constant<noexcept(declval<T&>().~T())> {};

template<class T, class Enable> struct is_polymorphic_impl                                                  : false_type {};
template<class T>               struct is_polymorphic_impl<T, decltype(dynamic_cast<void*>(declval<T*>()))> : true_type {};

} // namespace scratch::detail

namespace scratch {

template<class T, class... Us> struct is_constructible         : detail::is_constructible_impl<T, void, Us...> {};
template<class T, class... Us> struct is_nothrow_constructible : detail::is_nothrow_constructible_impl<T, void, Us...> {};
template<class T, class U>     struct is_assignable            : detail::is_assignable_impl<T, U, void> {};
template<class T, class U>     struct is_nothrow_assignable    : detail::is_nothrow_assignable_impl<T, U, void> {};
template<class T>              struct is_destructible          : detail::is_destructible_impl<remove_all_extents_t<T>, void> {};
template<class T>              struct is_nothrow_destructible  : detail::is_nothrow_destructible_impl<remove_all_extents_t<T>, void> {};
template<class T>              struct is_polymorphic           : detail::is_polymorphic_impl<remove_cv_t<T>, void*> {};

template<class T, class... Us> inline constexpr bool is_constructible_v         = is_constructible<T, Us...>::value;
template<class T, class... Us> inline constexpr bool is_nothrow_constructible_v = is_nothrow_constructible<T, Us...>::value;
template<class T, class U>     inline constexpr bool is_assignable_v            = is_assignable<T, U>::value;
template<class T, class U>     inline constexpr bool is_nothrow_assignable_v    = is_nothrow_assignable<T, U>::value;
template<class T>              inline constexpr bool is_destructible_v          = is_destructible<T>::value;
template<class T>              inline constexpr bool is_nothrow_destructible_v  = is_nothrow_destructible<T>::value;
template<class T>              inline constexpr bool is_polymorphic_v           = is_polymorphic<T>::value;

template<class T> struct is_default_constructible : is_constructible<T> {};
template<class T> struct is_nothrow_default_constructible : is_nothrow_constructible<T> {};
template<class T> inline constexpr bool is_default_constructible_v = is_default_constructible<T>::value;
template<class T> inline constexpr bool is_nothrow_default_constructible_v = is_nothrow_default_constructible<T>::value;

template<class T> struct is_copy_constructible : is_constructible<T, add_lvalue_reference_t<const T>> {};
template<class T> struct is_nothrow_copy_constructible : is_nothrow_constructible<T, add_lvalue_reference_t<const T>> {};
template<class T> inline constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;
template<class T> inline constexpr bool is_nothrow_copy_constructible_v = is_nothrow_copy_constructible<T>::value;

template<class T> struct is_move_constructible : is_constructible<T, add_rvalue_reference_t<T>> {};
template<class T> struct is_nothrow_move_constructible : is_nothrow_constructible<T, add_rvalue_reference_t<T>> {};
template<class T> inline constexpr bool is_move_constructible_v = is_move_constructible<T>::value;
template<class T> inline constexpr bool is_nothrow_move_constructible_v = is_nothrow_move_constructible<T>::value;

template<class T> struct is_copy_assignable : is_assignable<T, add_lvalue_reference_t<const T>> {};
template<class T> struct is_nothrow_copy_assignable : is_nothrow_assignable<T, add_lvalue_reference_t<const T>> {};
template<class T> inline constexpr bool is_copy_assignable_v = is_copy_assignable<T>::value;
template<class T> inline constexpr bool is_nothrow_copy_assignable_v = is_nothrow_copy_assignable<T>::value;

template<class T> struct is_move_assignable : is_assignable<T, add_rvalue_reference_t<T>> {};
template<class T> struct is_nothrow_move_assignable : is_nothrow_assignable<T, add_rvalue_reference_t<T>> {};
template<class T> inline constexpr bool is_move_assignable_v = is_move_assignable<T>::value;
template<class T> inline constexpr bool is_nothrow_move_assignable_v = is_nothrow_move_assignable<T>::value;

// The pseudo-function __has_trivial_destructor(T) is compiler magic.
template<class T> inline constexpr bool is_trivially_destructible_v = is_destructible_v<T> && __has_trivial_destructor(T);
template<typename T> struct is_trivially_destructible : bool_constant<is_trivially_destructible_v<T>> {};

// These "trivially_foo" functions can be implemented in terms of deeper magic.
template<class T> inline constexpr bool is_trivially_copy_assignable_v = is_trivially_assignable_v<T, add_lvalue_reference_t<const T>>;
template<class T> inline constexpr bool is_trivially_copy_constructible_v = is_trivially_constructible_v<T, add_lvalue_reference_t<const T>>;
template<class T> inline constexpr bool is_trivially_default_constructible_v = is_trivially_constructible_v<T>;
template<class T> inline constexpr bool is_trivially_move_assignable_v = is_trivially_assignable_v<T, add_rvalue_reference_t<T>>;
template<class T> inline constexpr bool is_trivially_move_constructible_v = is_trivially_constructible_v<T, add_rvalue_reference_t<T>>;
template<class T> inline constexpr bool is_trivial_v = is_trivially_copyable_v<T> && is_trivially_default_constructible_v<T>;

template<typename T> struct is_trivially_copy_assignable : bool_constant<is_trivially_copy_assignable_v<T>> {};
template<typename T> struct is_trivially_copy_constructible : bool_constant<is_trivially_copy_constructible_v<T>> {};
template<typename T> struct is_trivially_default_constructible : bool_constant<is_trivially_default_constructible_v<T>> {};
template<typename T> struct is_trivially_move_assignable : bool_constant<is_trivially_move_assignable_v<T>> {};
template<typename T> struct is_trivially_move_constructible : bool_constant<is_trivially_move_constructible_v<T>> {};
template<typename T> struct is_trivial : bool_constant<is_trivial_v<T>> {};

} // namespace scratch
