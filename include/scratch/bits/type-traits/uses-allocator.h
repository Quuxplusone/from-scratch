#pragma once

#include "scratch/bits/type-traits/erased-type-tag.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-convertible.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/priority-tag.h"
#include "scratch/bits/utility/declval.h"

namespace scratch::detail {

template<class T> auto uses_allocator_impl(const erased_type&, priority_tag<2>) -> true_type;
template<class T, class A> auto uses_allocator_impl(const A&, priority_tag<1>) -> is_convertible<A, typename T::allocator_type>;
template<class T, class A> auto uses_allocator_impl(const A&, priority_tag<0>) -> false_type;

} // namespace scratch::detail

namespace scratch {

template<class T, class A> struct uses_allocator : decltype(detail::uses_allocator_impl<T>(declval<const A&>(), priority_tag<2>{})) {};

template<class T, class A>
inline constexpr bool uses_allocator_v = uses_allocator<T, A>::value;

} // namespace scratch
