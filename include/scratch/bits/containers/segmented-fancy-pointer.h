#pragma once

#include "scratch/bits/containers/fancy-ptr-base.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-convertible.h"
#include "scratch/bits/type-traits/is-static-castable.h"
#include "scratch/bits/type-traits/is-foo.h"

#include <cstddef>

namespace scratch {

template<class T>
class segmented_fancy_pointer : public detail::fancy_ptr_base<T, segmented_fancy_pointer<T>> {
    T *m_ptr = nullptr;
    void *m_segment = nullptr;
public:
    segmented_fancy_pointer() = default;
    segmented_fancy_pointer(decltype(nullptr)) {}

    constexpr T *ptr() const noexcept { return m_ptr; }
    void *segment() const { return m_segment; }

    template<bool B = !is_void_v<T>, class = enable_if_t<B>>
    constexpr void increment_ptr(ptrdiff_t i) noexcept { m_ptr += i; }

    template<class U, bool_if_t<is_convertible_v<U*, T*>> = true>
    segmented_fancy_pointer(const segmented_fancy_pointer<U>& rhs) :
        segmented_fancy_pointer(rhs.segment(), rhs.ptr()) {}

    template<class U, bool_if_t<is_static_castable_v<T*, U*> && !is_convertible_v<U*, T*>> = true>
    explicit segmented_fancy_pointer(const segmented_fancy_pointer<U>& rhs) :
        segmented_fancy_pointer(rhs.segment(), static_cast<T *>(rhs.ptr())) {}

    explicit segmented_fancy_pointer(void *s, T *p) : m_segment(s) { this->m_ptr = p; }

    template<bool B = !is_void_v<T>>
    static auto pointer_to(enable_if_t<B, T>& r) {
        return segmented_fancy_pointer(nullptr, &r);
    }
};

} // namespace scratch
