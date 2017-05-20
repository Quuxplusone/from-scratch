#pragma once

#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/utility/declval.h"

namespace scratch::detail {

template <class B> true_type is_base_of_test(const volatile B*);
template <class B> false_type is_base_of_test(const volatile void*);

template <class B, class D, class Enable> struct is_base_of_impl                                                             : true_type {};
template <class B, class D>               struct is_base_of_impl<B, D, decltype(is_base_of_test<B>(declval<D*>()), (void)0)> : decltype(is_base_of_test<B>(declval<D*>())) {};

} // namespace scratch::detail

namespace scratch {

template<typename B, typename D> inline constexpr bool is_base_of_v =
    is_class_v<D> && detail::is_base_of_impl<B, D, void>::value;

template<class B, class D> struct is_base_of : bool_constant<is_base_of_v<B, D>> {};

} // namespace scratch
