#pragma once

#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/utility/declval.h"

namespace scratch::detail {

template<class T, class U, class Enable> struct is_static_castable_impl                                                    : false_type {};
template<class T, class U>               struct is_static_castable_impl<T, U, decltype(void(static_cast<U>(declval<T>()))> : true_type {};

} // namespace scratch::detail

namespace scratch {

template<typename From, typename To>
struct is_static_castable : public detail::is_static_castable_impl<From, To, void>::type {};

template<typename From, typename To>
inline constexpr bool is_static_castable_v = is_static_castable<From, To>::value;

} // namespace scratch
