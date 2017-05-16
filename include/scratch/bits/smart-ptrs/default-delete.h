#pragma once

#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-convertible.h"

namespace scratch {

template<typename T>
struct default_delete
{
    constexpr default_delete() noexcept = default;

    template<typename U, typename = enable_if_t<is_convertible_v<U*, T*>>>
    default_delete(const default_delete<U>&) noexcept {}

    void operator()(T *p) const {
        delete p;
    }
};

template<typename T>
struct default_delete<T[]>
{
    constexpr default_delete() noexcept = default;

    template<typename U, typename = enable_if_t<is_convertible_v<U(*)[], T(*)[]>>>
    default_delete(const default_delete<U[]>&) noexcept {}

    void operator()(T *p) const {
        delete [] p;
    }
};

} // namespace scratch
