#pragma once

#include "scratch/bits/smart-ptrs/default-delete.h"
#include "scratch/bits/smart-ptrs/smart-ptr-base.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-fooible.h"
#include "scratch/bits/utility/compressed-element.h"

#include <utility>

namespace scratch {

template<typename T, typename Deleter = default_delete<T>>
class unique_ptr : private compressed_element<Deleter, 0>, public detail::smart_ptr_base<T>
{
    using detail::smart_ptr_base<T>::m_ptr;
public:
    using element_type = typename detail::smart_ptr_base<T>::element_type;
    using pointer = element_type*;
    using deleter_type = Deleter;

    constexpr unique_ptr() noexcept = default;
    constexpr unique_ptr(decltype(nullptr)) noexcept {}
    constexpr unique_ptr(element_type *p) noexcept { m_ptr = p; }
    constexpr unique_ptr(element_type *p, Deleter d) noexcept : compressed_element<Deleter, 0>(std::move(d)) { m_ptr = p; }

    unique_ptr(unique_ptr&& rhs) noexcept : unique_ptr() {
        this->reset(rhs.release());
        get_deleter() = std::move(rhs.get_deleter());
    }

    unique_ptr& operator=(unique_ptr&& rhs) noexcept {
        reset(rhs.release());
        get_deleter() = std::move(rhs.get_deleter());
        return *this;
    }

    template<typename U, typename E, bool_if_t<is_convertible_v<U*, T*> && is_assignable_v<Deleter&, E&&>> = true>
    unique_ptr& operator=(unique_ptr<U,E>&& rhs) noexcept {
        reset(static_cast<T*>(rhs.release()));
        get_deleter() = std::move(rhs.get_deleter());
        return *this;
    }

    void swap(unique_ptr& rhs) noexcept {
        using std::swap;
        swap(m_ptr, rhs.m_ptr);
        swap(get_deleter(), rhs.get_deleter());
    }

    ~unique_ptr() {
        reset();
    }

    element_type *release() noexcept {
        return std::exchange(m_ptr, nullptr);
    }
    void reset(element_type *p = nullptr) noexcept {
        element_type *old_p = std::exchange(m_ptr, p);
        if (old_p != nullptr) {
            get_deleter()(old_p);
        }
    }

    Deleter& get_deleter() noexcept { return this->get_element(index_constant<0>{}); }
    const Deleter& get_deleter() const noexcept { return this->get_element(index_constant<0>{}); }
};

template<class T, class D> bool operator==(const unique_ptr<T,D>& a, decltype(nullptr)) noexcept { return a.get() == nullptr; }
template<class T, class D> bool operator!=(const unique_ptr<T,D>& a, decltype(nullptr)) noexcept { return a.get() != nullptr; }
template<class T, class D> bool operator==(decltype(nullptr), const unique_ptr<T,D>& a) noexcept { return a.get() == nullptr; }
template<class T, class D> bool operator!=(decltype(nullptr), const unique_ptr<T,D>& a) noexcept { return a.get() != nullptr; }

template<class T, class D, class U, class E> bool operator==(const unique_ptr<T,D>& a, const unique_ptr<U,E>& b) noexcept { return a.get() == b.get(); }
template<class T, class D, class U, class E> bool operator!=(const unique_ptr<T,D>& a, const unique_ptr<U,E>& b) noexcept { return a.get() != b.get(); }

template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace scratch
