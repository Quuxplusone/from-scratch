#pragma once

#include "scratch/bits/smart-ptrs/smart-ptr-base.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-fooible.h"

#include <utility>

namespace scratch {

template<class T>
class observer_ptr : public detail::smart_ptr_base<T>
{
    using detail::smart_ptr_base<T>::m_ptr;
public:
    using typename detail::smart_ptr_base<T>::element_type;

    constexpr observer_ptr() noexcept = default;
    constexpr observer_ptr(decltype(nullptr)) noexcept {}

    template<class U, bool_if_t<is_convertible_v<U*, element_type*>> = true>
    constexpr explicit observer_ptr(U *ptr) noexcept { m_ptr = ptr; }

    template<class U, bool_if_t<is_convertible_v<typename observer_ptr<U>::element_type*, element_type*>> = true>
    constexpr observer_ptr(observer_ptr<U> rhs) noexcept { m_ptr = rhs.get(); }

    constexpr void reset() noexcept { m_ptr = nullptr; }
    constexpr void reset(T *ptr) noexcept { m_ptr = ptr; }
    constexpr element_type *release() noexcept {
        return std::exchange(m_ptr, nullptr);
    }

    constexpr void swap(observer_ptr& rhs) noexcept { std::swap(m_ptr, rhs.m_ptr); }
};

template<class T, class U> bool operator==(observer_ptr<T> a, observer_ptr<U> b) { return a.get() == b.get(); }
template<class T, class U> bool operator!=(observer_ptr<T> a, observer_ptr<U> b) { return a.get() != b.get(); }
template<class T, class U> bool operator==(observer_ptr<T> a, decltype(nullptr)) { return a.get() == nullptr; }
template<class T, class U> bool operator!=(observer_ptr<T> a, decltype(nullptr)) { return a.get() != nullptr; }
template<class T, class U> bool operator==(decltype(nullptr), observer_ptr<U> b) { return nullptr == b.get(); }
template<class T, class U> bool operator!=(decltype(nullptr), observer_ptr<U> b) { return nullptr != b.get(); }

} // namespace scratch
