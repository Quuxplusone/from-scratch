#pragma once

#include "scratch/bits/type-traits/integral-constant.h"

namespace scratch {

template <typename T> class reference_wrapper;  // forward declaration

template<class T> struct is_reference_wrapper : false_type {};
template<class T> struct is_reference_wrapper<reference_wrapper<T>> : true_type {};
template<class T> inline constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

} // namespace scratch
