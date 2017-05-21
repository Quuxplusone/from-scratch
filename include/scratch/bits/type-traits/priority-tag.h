#pragma once

#include <cstddef>

namespace scratch {

// priority_tag is NOT a standard library type.
template<size_t I> struct priority_tag : priority_tag<I-1> {};
template<> struct priority_tag<0> {};

} // namespace scratch
