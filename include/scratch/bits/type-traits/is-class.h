#pragma once

#include "scratch/bits/type-traits/compiler-magic.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/priority-tag.h"

namespace scratch::detail {

template<typename T, typename = int T::*>
auto is_class_impl(priority_tag<1>) -> bool_constant<!is_union_v<T>>;

template<typename T>
auto is_class_impl(priority_tag<0>) -> false_type;

} // namespace scratch::detail

namespace scratch {

template<class T> struct is_class : decltype(detail::is_class_impl<T>(priority_tag<1>())) {};
template<class T> inline constexpr bool is_class_v = is_class<T>::value;

} // namespace scratch
