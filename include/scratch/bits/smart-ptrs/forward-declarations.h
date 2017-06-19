#pragma once

#include "scratch/bits/smart-ptrs/default-delete.h"

namespace scratch {

template<typename T, typename D = default_delete<T>> class unique_ptr;  // forward declaration

} // namespace scratch
