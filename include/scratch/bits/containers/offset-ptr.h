#pragma once

#include "scratch/bits/containers/fancy-ptr-base.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-convertible.h"
#include "scratch/bits/type-traits/is-static-castable.h"
#include "scratch/bits/type-traits/is-foo.h"

#include <cstddef>
#include <cstdint>

namespace scratch {

template<class T>
class offset_ptr : public detail::fancy_ptr_base<T, offset_ptr<T>> {
    uintptr_t m_ptr;
public:
    offset_ptr() = default;
    offset_ptr(decltype(nullptr)) { set_ptr(nullptr); }

    constexpr T *ptr() const noexcept { return (T *)m_ptr; }
    constexpr void set_ptr(T *p) noexcept { m_ptr = (uintptr_t)p; }

    template<bool B = !is_void_v<T>, class = enable_if_t<B>>
    constexpr void increment_ptr(ptrdiff_t i) noexcept { m_ptr += i * sizeof(T); }

    template<class U, bool_if_t<is_convertible_v<U*, T*>> = true>
    offset_ptr(const offset_ptr<U>& rhs) :
        offset_ptr(rhs.ptr()) {}

    template<class U, bool_if_t<is_static_castable_v<T*, U*> && !is_convertible_v<U*, T*>> = true>
    explicit offset_ptr(const offset_ptr<U>& rhs) :
        offset_ptr(static_cast<T *>(rhs.ptr())) {}

    explicit offset_ptr(T *p) { set_ptr(p); }

    template<bool B = !is_void_v<T>>
    static auto pointer_to(enable_if_t<B, T>& r) {
        return offset_ptr(&r);
    }
};

} // namespace scratch
