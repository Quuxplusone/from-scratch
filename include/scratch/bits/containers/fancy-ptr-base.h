#pragma once

#include <cstddef>

namespace scratch::detail {

template<class T, class CRTP>
struct fancy_ptr_base {
    constexpr T *ptr() const noexcept { return m_ptr; }
    constexpr explicit operator T*() const noexcept { return ptr(); }
    constexpr explicit operator bool() const noexcept { return ptr() != nullptr; }
    constexpr bool operator==(CRTP b) const { return ptr() == b.ptr(); }
    constexpr bool operator!=(CRTP b) const { return ptr() != b.ptr(); }
    constexpr bool operator==(decltype(nullptr)) const { return ptr() == nullptr; }
    constexpr bool operator!=(decltype(nullptr)) const { return ptr() != nullptr; }
    constexpr bool operator<(CRTP b) const { return ptr() < b.ptr(); }
    constexpr bool operator<=(CRTP b) const { return ptr() <= b.ptr(); }
    constexpr bool operator>(CRTP b) const { return ptr() > b.ptr(); }
    constexpr bool operator>=(CRTP b) const { return ptr() >= b.ptr(); }
    constexpr T& operator*() const noexcept { return *ptr(); }
    constexpr T* operator->() const noexcept { return ptr(); }
    constexpr CRTP& operator+=(ptrdiff_t i) { m_ptr += i; return as_crtp(); }
    constexpr CRTP& operator-=(ptrdiff_t i) { m_ptr -= i; return as_crtp(); }
    constexpr CRTP& operator++() { ++m_ptr; return as_crtp(); }
    constexpr CRTP& operator--() { --m_ptr; return as_crtp(); }
    constexpr CRTP operator++(int) { auto r(as_crtp()); ++*this; return r; }
    constexpr CRTP operator--(int) { auto r(as_crtp()); --*this; return r; }
    constexpr CRTP operator+(ptrdiff_t i) const { auto r(as_crtp()); r += i; return r; }
    constexpr CRTP operator-(ptrdiff_t i) const { auto r(as_crtp()); r -= i; return r; }
    constexpr ptrdiff_t operator-(CRTP b) const { return ptr() - b.ptr(); }

protected:
    T *m_ptr = nullptr;
private:
    constexpr CRTP& as_crtp() { return *static_cast<CRTP*>(this); }
    constexpr const CRTP& as_crtp() const { return *static_cast<const CRTP*>(this); }
};

template<class CRTP>
struct fancy_ptr_base<void, CRTP> {
    constexpr void *ptr() const noexcept { return m_ptr; }
    constexpr explicit operator void*() const noexcept { return ptr(); }
    constexpr explicit operator bool() const noexcept { return ptr() != nullptr; }
    constexpr bool operator==(CRTP b) const { return ptr() == b.ptr(); }
    constexpr bool operator!=(CRTP b) const { return ptr() != b.ptr(); }
    constexpr bool operator==(decltype(nullptr)) const { return ptr() == nullptr; }
    constexpr bool operator!=(decltype(nullptr)) const { return ptr() != nullptr; }
    constexpr bool operator<(CRTP b) const { return ptr() < b.ptr(); }
    constexpr bool operator<=(CRTP b) const { return ptr() <= b.ptr(); }
    constexpr bool operator>(CRTP b) const { return ptr() > b.ptr(); }
    constexpr bool operator>=(CRTP b) const { return ptr() >= b.ptr(); }
protected:
    void *m_ptr = nullptr;
};

} // namespace scratch::detail
