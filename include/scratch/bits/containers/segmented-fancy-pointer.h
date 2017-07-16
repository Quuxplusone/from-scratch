#pragma once

#include "scratch/bits/containers/fancy-ptr-base.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-convertible.h"
#include "scratch/bits/type-traits/is-explicitly-convertible.h"

namespace scratch {

template<class T>
class segmented_fancy_pointer : public detail::fancy_ptr_base<T, segmented_fancy_pointer<T>> {
    void *m_segment = nullptr;
public:
    segmented_fancy_pointer() = default;
    segmented_fancy_pointer(decltype(nullptr)) {}

    template<class U, bool_if_t<is_convertible_v<U*, T*>> = true>
    segmented_fancy_pointer(const segmented_fancy_pointer<U>& rhs) :
        segmented_fancy_pointer(rhs.segment(), rhs.ptr()) {}

    template<class U, bool_if_t<is_explicitly_convertible_v<T*, U*> && !is_convertible_v<U*, T*>> = true>
    explicit segmented_fancy_pointer(const segmented_fancy_pointer<U>& rhs) :
        segmented_fancy_pointer(rhs.segment(), static_cast<T *>(rhs.ptr())) {}

    explicit segmented_fancy_pointer(void *s, T *p) : m_segment(s) { this->m_ptr = p; }

    void *segment() const { return m_segment; }
};

} // namespace scratch
