#pragma once

#include <type_traits>

namespace scratch {

template<class T>
struct is_trivially_equality_comparable : std::bool_constant<
    std::is_integral<T>::value || std::is_pointer<T>::value
> {};

template<class T>
struct is_trivially_less_than_comparable : std::bool_constant<
    std::is_integral<T>::value || std::is_pointer<T>::value 
> {};

template<class T>
struct is_trivially_comparable : std::bool_constant<
    is_trivially_equality_comparable<T>::value && is_trivially_less_than_comparable<T>::value
> {};

} // namespace scratch
