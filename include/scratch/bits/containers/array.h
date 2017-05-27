#pragma once

#include "scratch/bits/algorithm/reverse-iterator.h"
#include "scratch/bits/stdexcept/stdexcept.h"
#include "scratch/bits/type-traits/is-swappable.h"

#include <cstddef>
#include <utility>

namespace scratch::detail {

template<class T, size_t N>
struct array_storage {
    using type = T[N];
};

template<class T>
struct array_storage<T, 0> {
    struct alignas(T) type {
        constexpr T& operator[](size_t) { return *reinterpret_cast<T*>(this); }
        constexpr const T& operator[](size_t) const { return *reinterpret_cast<const T*>(this); }
    };
};

template<class T, size_t N>
using array_storage_t = typename array_storage<T, N>::type;

} // namespace scratch::detail

namespace scratch {

template<class T, size_t N>
struct array {
    // This member must be public, so that is_aggregate_v<array<int, 2>>
    detail::array_storage_t<T, N> m_data;

    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = scratch::reverse_iterator<iterator>;
    using const_reverse_iterator = scratch::reverse_iterator<const_iterator>;

    constexpr T& at(size_t i) {
        if (i >= N) throw out_of_range(exception::nocopy, "at");
        return m_data[i];
    }

    constexpr const T& at(size_t i) const {
        if (i >= N) throw out_of_range(exception::nocopy, "at");
        return m_data[i];
    }

    constexpr T& operator[](size_t i) { return m_data[i]; }
    constexpr const T& operator[](size_t i) const { return m_data[i]; }

    constexpr T* data() noexcept { return m_data; }
    constexpr const T* data() const noexcept { return m_data; }

    constexpr T& front() { return m_data[0]; }
    constexpr const T& front() const { return m_data[0]; }
    constexpr T& back() { return m_data[size()-1]; }
    constexpr const T& back() const { return m_data[size()-1]; }

    constexpr iterator begin() noexcept { return &m_data[0]; }
    constexpr const_iterator cbegin() noexcept { return &m_data[0]; }
    constexpr const_iterator begin() const noexcept { return &m_data[0]; }
    constexpr iterator end() noexcept { return &m_data[size()]; }
    constexpr const_iterator cend() noexcept { return &m_data[size()]; }
    constexpr const_iterator end() const noexcept { return &m_data[size()]; }

    constexpr size_t size() const noexcept { return N; }
    constexpr bool empty() const noexcept { return size() == 0; }

    // This container is not resizeable, so "max size" is trivial.
    constexpr size_t max_size() const noexcept { return N; }

    void swap(array& rhs) noexcept(is_nothrow_swappable_v<T>) {
        std::swap(m_data, rhs.m_data);
    }
};

template<class T, size_t N>
void swap(array<T, N>& lhs, array<T, N>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

template<size_t I, class T, size_t N> constexpr T& get(array<T, N>& a) noexcept { static_assert(I < N); return a[I]; }
template<size_t I, class T, size_t N> constexpr T&& get(array<T, N>&& a) noexcept { static_assert(I < N); return a[I]; }
template<size_t I, class T, size_t N> constexpr const T& get(const array<T, N>& a) noexcept { static_assert(I < N); return a[I]; }
template<size_t I, class T, size_t N> constexpr const T&& get(const array<T, N>&& a) noexcept { static_assert(I < N); return a[I]; }

} // namespace scratch
