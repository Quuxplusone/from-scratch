#pragma once

#include <cstddef>

namespace my::detail {

template<class T>
struct smart_ptr_base {
    using element_type = T;
    constexpr T *get() const noexcept { return m_ptr; }
    constexpr explicit operator bool() const noexcept { return get() != nullptr; }
    constexpr T& operator*() const noexcept { return *get(); }
    constexpr T* operator->() const noexcept { return get(); }
protected:
    T *m_ptr = nullptr;
};

template<class T>
struct smart_ptr_base<T[]> {
    using element_type = T;
    constexpr T *get() const noexcept { return m_ptr; }
    constexpr explicit operator bool() const noexcept { return get() != nullptr; }
    constexpr T& operator[](ptrdiff_t i) const noexcept { return get()[i]; }
protected:
    T *m_ptr = nullptr;
};

template<>
struct smart_ptr_base<void> {
    using element_type = void;
    constexpr void *get() const noexcept { return m_ptr; }
    constexpr explicit operator bool() const noexcept { return get() != nullptr; }
protected:
    void *m_ptr = nullptr;
};

} // namespace my::detail


