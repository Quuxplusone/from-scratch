#pragma once

#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/utility/declval.h"

#include <utility>

namespace scratch::detail::swappable {

using std::swap;

template<class T, class U, class Enable> struct is_oneway_swappable_impl                                                                : false_type {};
template<class T, class U>               struct is_oneway_swappable_impl<T, U, decltype(swap(declval<T>(), declval<U>()), (void)0)>     : true_type {};

template<class T, class U, class Enable> struct is_nothrow_oneway_swappable_impl                                                        : false_type {};
template<class T, class U>               struct is_nothrow_oneway_swappable_impl<T, U, decltype(swap(declval<T>(), declval<U>()), (void)0)> : bool_constant<noexcept(swap(declval<T>(), declval<U>()))> {};

} // namespace scratch::detail::swappable

namespace scratch {

template<class T, class U> inline constexpr bool is_swappable_with_v =
    detail::swappable::is_oneway_swappable_impl<T, U, void>::value && detail::swappable::is_oneway_swappable_impl<U, T, void>::value;
template<class T, class U> inline constexpr bool is_nothrow_swappable_with_v =
    detail::swappable::is_nothrow_oneway_swappable_impl<T, U, void>::value && detail::swappable::is_nothrow_oneway_swappable_impl<U, T, void>::value;

template<class T> inline constexpr bool is_swappable_v         = is_swappable_with_v<T, T>;
template<class T> inline constexpr bool is_nothrow_swappable_v = is_nothrow_swappable_with_v<T, T>;

template<class T, class U> struct is_swappable_with         : bool_constant<is_swappable_with_v<T, U>> {};
template<class T, class U> struct is_nothrow_swappable_with : bool_constant<is_nothrow_swappable_with_v<T, U>> {};

template<class T>          struct is_swappable              : bool_constant<is_swappable_v<T>> {};
template<class T>          struct is_nothrow_swappable      : bool_constant<is_nothrow_swappable_v<T>> {};

} // namespace scratch
