#pragma once

#include "scratch/bits/aligned-storage/max-align-t.h"

#include <cstddef>

namespace scratch {

template<size_t Len, size_t Align = alignof(max_align_t)>
struct aligned_storage {
    using type = struct {
        alignas(Align) char data[Len];
    };

    virtual void you_probably_meant_to_use_the_member_typedef_type() = 0;
};

template<size_t Len, size_t Align = alignof(max_align_t)>
using aligned_storage_t = typename aligned_storage<Len, Align>::type;

} // namespace scratch
