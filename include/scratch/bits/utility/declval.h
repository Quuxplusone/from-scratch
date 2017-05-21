#pragma once

#include "scratch/bits/type-traits/add-foo.h"

namespace scratch {

template <class T> auto declval() noexcept -> add_rvalue_reference_t<T>;

} // namespace scratch
