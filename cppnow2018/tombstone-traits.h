#pragma once

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

template<class> class optional;
template<bool B> using bool_if_t = std::enable_if_t<B, bool>;

template<class T>
struct tombstone_traits<optional<T>>
{
    static constexpr size_t spare_representations =
        (tombstone_traits<T>::spare_representations >= 1) ?
        (tombstone_traits<T>::spare_representations - 1) :
        tombstone_traits<bool>::spare_representations;

    template<class T_ = T, bool_if_t<tombstone_traits<T_>::spare_representations >= 1> = true>
    static constexpr void set_spare_representation(optional<T> *p, size_t idx) {
        // representation 0 is being used for "nullopt", so add 1
        tombstone_traits<T>::set_spare_representation(&p->m_value, idx + 1);
    }
    template<class T_ = T, bool_if_t<tombstone_traits<T_>::spare_representations >= 1> = true>
    static constexpr size_t index(const optional<T> *p) {
        // representation 0 is "nullopt", so it's a valid representation
        auto i = tombstone_traits<T>::index(&p->m_value);
        return (i == size_t(-1) || i == 0) ? -1 : (i - 1);
    }
    template<class T_ = T, bool_if_t<tombstone_traits<T_>::spare_representations == 0> = true>
    static constexpr void set_spare_representation(optional<T> *p, size_t idx) {
        tombstone_traits<bool>::set_spare_representation(&p->m_has_value, idx);
    }
    template<class T_ = T, bool_if_t<tombstone_traits<T_>::spare_representations == 0> = true>
    static constexpr size_t index(const optional<T> *p) {
        return tombstone_traits<bool>::index(&p->m_has_value);
    }
};

} // namespace scratch
