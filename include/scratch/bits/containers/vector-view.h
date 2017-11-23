#pragma once

#include "scratch/bits/algorithm/advance.h"
#include "scratch/bits/stdexcept/stdexcept.h"
#include "scratch/bits/traits-classes/pointer-traits.h"

#include <cstddef>
#include <utility>

namespace scratch {

template<class T, class VoidPtr = void*>
class vector_view {
    using Ptr_traits = pointer_traits<VoidPtr>;
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = typename Ptr_traits::difference_type;
    using reference = T&;
    using const_reference = const T&;
    using pointer = typename Ptr_traits::template rebind<T>;
    using const_pointer = typename Ptr_traits::template rebind<const T>;
    using iterator = pointer;
    using const_iterator = const_pointer;

    vector_view() noexcept : m_data(pointer{}) {}
    explicit vector_view(pointer first, pointer last) : m_data(first), m_size(last - first) {}
    template<class It> explicit vector_view(It first, It last) : m_data(&*first), m_size(scratch::distance(first, last)) {}

    vector_view(vector_view&&) noexcept = default;
    vector_view(const vector_view&) noexcept = default;
    vector_view& operator=(vector_view&&) noexcept = default;
    vector_view& operator=(const vector_view&) noexcept = default;

    void swap(vector_view& rhs) noexcept {
        using std::swap;
        swap(this->m_data, rhs.m_data);
        swap(this->m_size, rhs.m_size);
    }

    constexpr T& at(size_t i) {
        if (i >= size()) throw out_of_range(exception::nocopy, "at");
        return data()[i];
    }

    constexpr const T& at(size_t i) const {
        if (i >= size()) throw out_of_range(exception::nocopy, "at");
        return data()[i];
    }

    constexpr T& operator[](size_t i) { return data()[i]; }
    constexpr const T& operator[](size_t i) const { return data()[i]; }

    constexpr T* data() noexcept { return static_cast<T*>(m_data); }
    constexpr const T* data() const noexcept { return static_cast<T*>(m_data); }

    constexpr T& front() { return data()[0]; }
    constexpr const T& front() const { return data()[0]; }
    constexpr T& back() { return data()[size()-1]; }
    constexpr const T& back() const { return data()[size()-1]; }

    constexpr iterator begin() noexcept { return &data()[0]; }
    constexpr const_iterator cbegin() noexcept { return &data()[0]; }
    constexpr const_iterator begin() const noexcept { return &data()[0]; }
    constexpr iterator end() noexcept { return &data()[size()]; }
    constexpr const_iterator cend() noexcept { return &data()[size()]; }
    constexpr const_iterator end() const noexcept { return &data()[size()]; }

    constexpr size_t size() const noexcept { return m_size; }
    constexpr bool empty() const noexcept { return size() == 0; }

private:
    pointer m_data;
    size_t m_size = 0;
};

template<class T, class VP>
void swap(vector_view<T, VP>& lhs, vector_view<T, VP>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace scratch
