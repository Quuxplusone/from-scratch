#pragma once

#include "scratch/bits/type-traits/enable-if.h"

#include <cstddef>

namespace scratch {

template<class T>
struct tombstone_traits {
    static constexpr size_t spare_representations = 0;
    static constexpr void set_spare_representation(T *, size_t) = delete;
    static constexpr size_t index(const T *) { return size_t(-1); }
};

template<>
struct tombstone_traits<bool> {
    static constexpr size_t spare_representations = 254;
    static constexpr void set_spare_representation(bool *p, size_t idx) {
        *(unsigned char *)(p) = (idx + 2);
    }
    static constexpr size_t index(const bool *p) {
        size_t v = *(const unsigned char *)(p);
        if (v == 0 || v == 1) {
            return size_t(-1);
        } else {
            return v - 2;
        }
    }
};

} // namespace scratch
