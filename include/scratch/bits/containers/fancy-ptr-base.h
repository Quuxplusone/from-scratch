#pragma once

#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-foo.h"

#include <cstddef>

namespace scratch::detail {

template<class T, class CRTP, class = void>
struct fancy_ptr_base {
    constexpr T *ptr() const noexcept { return as_crtp().ptr(); }
    constexpr explicit operator T*() const noexcept { return ptr(); }
    constexpr explicit operator bool() const noexcept { return ptr() != nullptr; }
    constexpr bool operator==(CRTP b) const noexcept { return ptr() == b.ptr(); }
    constexpr bool operator!=(CRTP b) const noexcept { return ptr() != b.ptr(); }
    constexpr bool operator==(decltype(nullptr)) const noexcept { return ptr() == nullptr; }
    constexpr bool operator!=(decltype(nullptr)) const noexcept { return ptr() != nullptr; }
    constexpr bool operator<(CRTP b) const noexcept { return ptr() < b.ptr(); }
    constexpr bool operator<=(CRTP b) const noexcept { return ptr() <= b.ptr(); }
    constexpr bool operator>(CRTP b) const noexcept { return ptr() > b.ptr(); }
    constexpr bool operator>=(CRTP b) const noexcept { return ptr() >= b.ptr(); }
    constexpr T& operator*() const noexcept { return *ptr(); }
    constexpr T* operator->() const noexcept { return ptr(); }
    constexpr T& operator[](ptrdiff_t i) const noexcept { return ptr()[i]; }
    constexpr CRTP& operator+=(ptrdiff_t i) noexcept { as_crtp().increment_ptr(i); return as_crtp(); }
    constexpr CRTP& operator-=(ptrdiff_t i) noexcept { as_crtp().increment_ptr(-i); return as_crtp(); }
    constexpr CRTP& operator++() noexcept { as_crtp().increment_ptr(1); return as_crtp(); }
    constexpr CRTP& operator--() noexcept { as_crtp().increment_ptr(-1); return as_crtp(); }
    constexpr CRTP operator++(int) noexcept { auto r(as_crtp()); ++*this; return r; }
    constexpr CRTP operator--(int) noexcept { auto r(as_crtp()); --*this; return r; }
    constexpr CRTP operator+(ptrdiff_t i) const noexcept { auto r(as_crtp()); r += i; return r; }
    constexpr CRTP operator-(ptrdiff_t i) const noexcept { auto r(as_crtp()); r -= i; return r; }
    constexpr ptrdiff_t operator-(CRTP b) const noexcept { return ptr() - b.ptr(); }

private:
    constexpr CRTP& as_crtp() { return *static_cast<CRTP*>(this); }
    constexpr const CRTP& as_crtp() const { return *static_cast<const CRTP*>(this); }
};

template<class T, class CRTP>
struct fancy_ptr_base<T, CRTP, enable_if_t<is_void_v<T>>> {
    constexpr T *ptr() const noexcept { return as_crtp().ptr(); }
    constexpr explicit operator T*() const noexcept { return ptr(); }
    constexpr explicit operator bool() const noexcept { return ptr() != nullptr; }
    constexpr bool operator==(CRTP b) const noexcept { return ptr() == b.ptr(); }
    constexpr bool operator!=(CRTP b) const noexcept { return ptr() != b.ptr(); }
    constexpr bool operator==(decltype(nullptr)) const noexcept { return ptr() == nullptr; }
    constexpr bool operator!=(decltype(nullptr)) const noexcept { return ptr() != nullptr; }
    constexpr bool operator<(CRTP b) const noexcept { return ptr() < b.ptr(); }
    constexpr bool operator<=(CRTP b) const noexcept { return ptr() <= b.ptr(); }
    constexpr bool operator>(CRTP b) const noexcept { return ptr() > b.ptr(); }
    constexpr bool operator>=(CRTP b) const noexcept { return ptr() >= b.ptr(); }

private:
    constexpr CRTP& as_crtp() { return *static_cast<CRTP*>(this); }
    constexpr const CRTP& as_crtp() const { return *static_cast<const CRTP*>(this); }
};

} // namespace scratch::detail
