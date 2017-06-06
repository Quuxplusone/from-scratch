#pragma once

#include "scratch/bits/traits-classes/tombstone-traits.h"

#include <cstddef>

namespace scratch {

template<class T>
struct tombstone_traits<optional<T>>
{
    static constexpr size_t spare_representations =
        (tombstone_traits<T>::spare_representations >= 1) ?
        (tombstone_traits<T>::spare_representations - 1) :
        tombstone_traits<bool>::spare_representations;

    static constexpr void set_spare_representation(optional<T> *p, size_t idx) {
        if constexpr (tombstone_traits<T>::spare_representations >= 1) {
            // representation 0 is being used for "nullopt", so add 1
            tombstone_traits<T>::set_spare_representation(&p->m_value, idx + 1);
        } else {
            tombstone_traits<bool>::set_spare_representation(&p->m_has_value, idx);
        }
    }
    static constexpr size_t index(const optional<T> *p) {
        if constexpr (tombstone_traits<T>::spare_representations >= 1) {
            // representation 0 is "nullopt", so it's a valid representation
            auto i = tombstone_traits<T>::index(&p->m_value);
            return (i == size_t(-1) || i == 0) ? -1 : (i - 1);
        } else {
            return tombstone_traits<bool>::index(&p->m_has_value);
        }
    }
};

} // namespace scratch
