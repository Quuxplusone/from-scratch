#pragma once

#include <cstddef>

namespace scratch {

template<class T>
struct counted_copying_iterator {
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = const T *;
    using reference = const T&;
    using iterator_category = random_access_iterator_tag;

    explicit counted_copying_iterator() noexcept = default;
    explicit counted_copying_iterator(size_t c, const T& t) noexcept : m_count(c), m_t(&t) {}

    const T& operator*() const noexcept { return *m_t; }
    const T *operator->() const noexcept { return m_t; }
    const T& operator[](difference_type) const noexcept { return m_t; }

    auto& operator+=(int i) noexcept { m_count -= i; return *this; }
    auto& operator++() noexcept { return *this += 1; }
    auto operator++(int) noexcept { auto old = *this; ++*this; return old; }

    auto& operator-=(int i) noexcept { m_count += i; return *this; }
    auto& operator--() noexcept { return *this -= 1; }
    auto operator--(int) noexcept { auto old = *this; --*this; return old; }

    difference_type operator-(const counted_copying_iterator& rhs) const noexcept { return rhs.m_count - m_count; }

    bool operator==(const counted_copying_iterator& rhs) const noexcept { return m_count == rhs.m_count; }
    bool operator!=(const counted_copying_iterator& rhs) const noexcept { return m_count != rhs.m_count; }

private:
    size_t m_count = 0;
    const T *m_t = nullptr;
};

template<class T> auto operator-(counted_copying_iterator<T> a, ptrdiff_t b) noexcept { a -= b; return a; }
template<class T> auto operator+(counted_copying_iterator<T> a, ptrdiff_t b) noexcept { a += b; return a; }
template<class T> auto operator+(ptrdiff_t b, counted_copying_iterator<T> a) noexcept { a += b; return a; }

} // namespace scratch
