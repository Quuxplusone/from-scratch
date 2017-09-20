#pragma once

#include <cstddef>

namespace scratch {

template<class> struct tuple_size;  // forward declaration
template<class, size_t> struct tuple_element;  // forward declaration

template<class T> inline constexpr size_t tuple_size_v = tuple_size<T>::value;
template<class T, size_t I> using tuple_element_t = typename tuple_element<T, I>::type;

} // namespace scratch
