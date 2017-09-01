#pragma once

#include "scratch/bits/type-traits/is-foo.h"

namespace scratch {

template<class It> struct is_contiguous_iterator : is_pointer<It> {};

template<class It> inline constexpr bool is_contiguous_iterator_v = is_contiguous_iterator<It>::value;

} // namespace scratch
