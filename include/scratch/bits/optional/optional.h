#pragma once

#include "scratch/bits/stdexcept/bad-optional-access.h"
#include "scratch/bits/optional/enable-optional-smfs.h"
#include "scratch/bits/optional/in-place.h"
#include "scratch/bits/optional/nullopt.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-convertible.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-fooible.h"
#include "scratch/bits/type-traits/is-swappable.h"

#include <initializer_list>
#include <new>
#include <utility>

namespace scratch {

template<class T>
class optional {

    static_assert(is_object_v<T>, "optional<T> works only with object types");

    union {
        char m_dummy;
        T m_value;
    };
    bool m_has_value;

public:
    using value_type = T;

    constexpr optional() noexcept : m_has_value(false) {}
    constexpr optional(nullopt_t) noexcept : m_has_value(false) {}

    template<bool_if_t<is_copy_constructible_v<T>> = true>
    optional(const optional& rhs)
        noexcept(is_nothrow_copy_constructible_v<T>)
    {
        m_has_value = rhs.has_value();
        if (m_has_value) {
            ::new (&m_value) T(*rhs);
        }
    }

    template<bool_if_t<is_move_constructible_v<T>> = true>
    optional(optional&& rhs)
        noexcept(is_nothrow_move_constructible_v<T>)
    {
        m_has_value = rhs.has_value();
        if (m_has_value) {
            ::new (&m_value) T(std::move(*rhs));
        }
    }

    template<class U, bool_if_t<is_constructible_v<T, const U&> && !is_convertible_v<const U&, T>> = true>
    explicit optional(const optional<U>& rhs)
        noexcept(is_nothrow_constructible_v<T, const U&>)
    {
        m_has_value = rhs.has_value();
        if (m_has_value) {
            ::new (&m_value) T(*rhs);
        }
    }

    template<class U, bool_if_t<is_constructible_v<T, const U&> && is_convertible_v<const U&, T>> = true>
    optional(const optional<U>& rhs)
        noexcept(is_nothrow_constructible_v<T, const U&>)
    {
        m_has_value = rhs.has_value();
        if (m_has_value) {
            ::new (&m_value) T(*rhs);
        }
    }

    template<class U, bool_if_t<is_constructible_v<T, U&&> && !is_convertible_v<U&&, T>> = true>
    explicit optional(optional<U>&& rhs)
        noexcept(is_nothrow_constructible_v<T, U&&>)
    {
        m_has_value = rhs.has_value();
        if (m_has_value) {
            ::new (&m_value) T(std::move(*rhs));
        }
    }

    template<class U, bool_if_t<is_constructible_v<T, U&&> && is_convertible_v<U&&, T>> = true>
    optional(optional<U>&& rhs)
        noexcept(is_nothrow_constructible_v<T, U&&>)
    {
        m_has_value = rhs.has_value();
        if (m_has_value) {
            ::new (&m_value) T(std::move(*rhs));
        }
    }

    template<class U = T, bool_if_t<is_constructible_v<T, U&&> && !is_convertible_v<U&&, T>> = true>
    explicit optional(U&& value)
        noexcept(is_nothrow_constructible_v<T, U&&>)
    {
        m_has_value = true;
        ::new (&m_value) T(std::forward<U>(value));
    }

    template<class U = T, bool_if_t<is_constructible_v<T, U&&> && is_convertible_v<U&&, T>> = true>
    optional(U&& value)
        noexcept(is_nothrow_constructible_v<T, U&&>)
    {
        m_has_value = true;
        ::new (&m_value) T(std::forward<U>(value));
    }

    template<class... Args, bool_if_t<is_constructible_v<T, Args&&...>> = true>
    explicit optional(in_place_t, Args&&... args)
        noexcept(is_nothrow_constructible_v<T, Args&&...>)
    {
        m_has_value = true;
        ::new (&m_value) T(std::forward<Args>(args)...);
    }

    template<class U, class... Args, bool_if_t<is_constructible_v<T, std::initializer_list<U>&, Args&&...>> = true>
    explicit optional(in_place_t, std::initializer_list<U> il, Args&&... args)
        noexcept(is_nothrow_constructible_v<T, std::initializer_list<U>&, Args&&...>)
    {
        m_has_value = true;
        ::new (&m_value) T(il, std::forward<Args>(args)...);
    }

    optional& operator=(nullopt_t) noexcept
    {
        reset();
        return *this;
    }

    template<class U, bool_if_t<is_constructible_v<T, const U&> && is_assignable_v<T&, const U&> && detail::enable_assignment_of_optional_from_optional_v<T, U>> = true>
    optional& operator=(const optional<U>& rhs)
        noexcept(is_nothrow_constructible_v<T, const U&> && is_nothrow_assignable_v<T&, const U&>)
    {
        if (rhs.has_value()) {
            if (this->has_value()) {
                m_value = *rhs;
            } else {
                m_has_value = true;
                ::new (&m_value) T(*rhs);
            }
        } else {
            reset();
        }
        return *this;
    }

    template<class U, bool_if_t<is_constructible_v<T, U&&> && is_assignable_v<T&, U&&> && detail::enable_assignment_of_optional_from_optional_v<T, U>> = true>
    optional& operator=(optional<U>&& rhs)
        noexcept(is_nothrow_constructible_v<T, U&&> && is_nothrow_assignable_v<T&, U&&>)
    {
        if (rhs.has_value()) {
            if (this->has_value()) {
                m_value = std::move(*rhs);
            } else {
                m_has_value = true;
                ::new (&m_value) T(std::move(*rhs));
            }
        } else {
            reset();
        }
        return *this;
    }

    template<class U = T, bool_if_t<is_constructible_v<T, U&&> && is_assignable_v<T&, U&&> && detail::enable_assignment_of_optional_from_forwarded_v<T, U>> = true>
    optional& operator=(U&& rhs)
        noexcept(is_nothrow_constructible_v<T, U&&> && is_nothrow_assignable_v<T&, U&&>)
    {

        if (this->has_value()) {
            m_value = std::forward<U>(rhs);
        } else {
            m_has_value = true;
            ::new (&m_value) T(std::forward<U>(rhs));
        }
        return *this;
    }

    ~optional() {
        reset();
    }

    constexpr const T* operator->() const   { return &m_value; }
    constexpr T* operator->()               { return &m_value; }
    constexpr const T& operator*() const&   { return m_value; }
    constexpr T& operator*() &              { return m_value; }
    constexpr const T&& operator*() const&& { return std::move(m_value); }
    constexpr T&& operator*() &&            { return std::move(m_value); }

    constexpr explicit operator bool() const noexcept { return has_value(); }
    constexpr bool has_value() const noexcept { return m_has_value; }

    constexpr T& value() &               { if (!has_value()) throw bad_optional_access(); return *(*this); }
    constexpr const T& value() const &   { if (!has_value()) throw bad_optional_access(); return *(*this); }
    constexpr T&& value() &&             { if (!has_value()) throw bad_optional_access(); return std::move(*(*this)); }
    constexpr const T&& value() const && { if (!has_value()) throw bad_optional_access(); return std::move(*(*this)); }

    template<class U = T> constexpr T value_or(U&& u) const & { return has_value() ? *(*this) : static_cast<T>(std::forward<U>(u)); }
    template<class U = T> constexpr T value_or(U&& u) && { return has_value() ? std::move(*(*this)) : static_cast<T>(std::forward<U>(u)); }

    void swap(optional& rhs)
        noexcept(is_nothrow_move_constructible_v<T> && is_nothrow_swappable_v<T>)
    {
        using std::swap;
        if (has_value()) {
            if (rhs.has_value()) {
                swap(**this, *rhs);
            } else {
                ::new (&rhs.m_value) T(std::move(m_value));
                rhs.m_has_value = true;
                m_value.~T();
                m_has_value = false;
            }
        } else {
            if (rhs.has_value()) {
                ::new (&m_value) T(std::move(rhs.m_value));
                m_has_value = true;
                rhs.m_value.~T();
                rhs.m_has_value = false;
            } else {
                // do nothing
            }
        }
    }

    void reset() noexcept {
        if (m_has_value) {
            m_value.~T();
            m_has_value = false;
        }
    }

    template<class... Args, bool_if_t<is_constructible_v<T, Args&&...>> = true>
    T& emplace(Args&&... args)
        noexcept(is_nothrow_constructible_v<T, Args&&...>)
    {
        reset();
        ::new (&m_value) T(std::forward<Args>(args)...);
        m_has_value = true;
        return m_value;
    }

    template<class U, class... Args, bool_if_t<is_constructible_v<T, std::initializer_list<U>&, Args&&...>> = true>
    T& emplace(std::initializer_list<U> il, Args&&... args)
        noexcept(is_nothrow_constructible_v<T, std::initializer_list<U>&, Args&&...>)
    {
        reset();
        ::new (&m_value) T(il, std::forward<Args>(args)...);
        m_has_value = true;
        return m_value;
    }
};

template<class T>
void swap(optional<T>& a, optional<T>& b)
    noexcept(noexcept(a.swap(b)))
{
    a.swap(b);
}

template<class T, class U> constexpr bool operator==(const optional<T>& a, const optional<U>& b) { return bool(a) ? (bool(b) ? (*a == *b) : false) : !bool(b); }
template<class T, class U> constexpr bool operator!=(const optional<T>& a, const optional<U>& b) { return bool(a) ? (bool(b) ? (*a != *b) : true) : bool(b); }
template<class T, class U> constexpr bool operator<(const optional<T>& a, const optional<U>& b) { return bool(a) ? (bool(b) ? (*a < *b) : false) : bool(b); }
template<class T, class U> constexpr bool operator<=(const optional<T>& a, const optional<U>& b) { return bool(a) ? (bool(b) ? (*a <= *b) : false) : true; }
template<class T, class U> constexpr bool operator>(const optional<T>& a, const optional<U>& b) { return bool(a) ? (bool(b) ? (*a > *b) : true) : false; }
template<class T, class U> constexpr bool operator>=(const optional<T>& a, const optional<U>& b) { return bool(a) ? (bool(b) ? (*a >= *b) : true) : !bool(b); }

template<class T, class U> constexpr bool operator==(const optional<T>& a, const U& b) { return bool(a) ? (*a == b) : false; }
template<class T, class U> constexpr bool operator!=(const optional<T>& a, const U& b) { return bool(a) ? (*a != b) : true; }
template<class T, class U> constexpr bool operator<(const optional<T>& a, const U& b) { return bool(a) ? (*a < b) : true; }
template<class T, class U> constexpr bool operator<=(const optional<T>& a, const U& b) { return bool(a) ? (*a <= b) : true; }
template<class T, class U> constexpr bool operator>(const optional<T>& a, const U& b) { return bool(a) ? (*a > b) : false; }
template<class T, class U> constexpr bool operator>=(const optional<T>& a, const U& b) { return bool(a) ? (*a >= b) : false; }

template<class T, class U> constexpr bool operator==(const U& b, const optional<T>& a) { return bool(a) ? (b == *a) : false; }
template<class T, class U> constexpr bool operator!=(const U& b, const optional<T>& a) { return bool(a) ? (b != *a) : true; }
template<class T, class U> constexpr bool operator<(const U& b, const optional<T>& a) { return bool(a) ? (b < *a) : false; }
template<class T, class U> constexpr bool operator<=(const U& b, const optional<T>& a) { return bool(a) ? (b <= *a) : false; }
template<class T, class U> constexpr bool operator>(const U& b, const optional<T>& a) { return bool(a) ? (b > *a) : true; }
template<class T, class U> constexpr bool operator>=(const U& b, const optional<T>& a) { return bool(a) ? (b >= *a) : true; }

template<class T> constexpr bool operator==(const optional<T>& a, nullopt_t) { return !bool(a); }
template<class T> constexpr bool operator!=(const optional<T>& a, nullopt_t) { return bool(a); }
template<class T> constexpr bool operator<(const optional<T>&, nullopt_t) { return false; }
template<class T> constexpr bool operator<=(const optional<T>& a, nullopt_t) { return !bool(a); }
template<class T> constexpr bool operator>(const optional<T>& a, nullopt_t) { return bool(a); }
template<class T> constexpr bool operator>=(const optional<T>&, nullopt_t) { return true; }

template<class T> constexpr bool operator==(nullopt_t, const optional<T>& a) { return !bool(a); }
template<class T> constexpr bool operator!=(nullopt_t, const optional<T>& a) { return bool(a); }
template<class T> constexpr bool operator<(nullopt_t, const optional<T>& a) { return bool(a); }
template<class T> constexpr bool operator<=(nullopt_t, const optional<T>&) { return true; }
template<class T> constexpr bool operator>(nullopt_t, const optional<T>&) { return false; }
template<class T> constexpr bool operator>=(nullopt_t, const optional<T>& a) { return !bool(a); }

} // namespace scratch
