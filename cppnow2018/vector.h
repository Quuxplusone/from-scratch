#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include "is-relocatable.h"
#include "polyfill.h"

namespace scratch {

using std::allocator;
using std::allocator_traits;
using std::get;
using std::invalid_argument;
using std::length_error;
using std::pair;

template<class T, class Alloc = allocator<T>>
class vector {
    using Alloc_traits = allocator_traits<Alloc>;
public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = typename Alloc_traits::size_type;
    using difference_type = typename Alloc_traits::difference_type;
    using reference = T&;
    using const_reference = const T&;
    using pointer = typename Alloc_traits::pointer;
    using const_pointer = typename Alloc_traits::const_pointer;
    using iterator = T*;
    using const_iterator = const T*;

    vector() noexcept : vector(Alloc{}) {}
    explicit vector(size_t count) : vector(count, Alloc{}) {}
    explicit vector(size_t count, const T& value) : vector(count, value, Alloc{}) {}
    template<class It> explicit vector(It first, It last) : vector(first, last, Alloc{}) {}

    explicit vector(Alloc a) noexcept : m_data_and_allocator(pointer{}, std::move(a)) {}
    explicit vector(size_t count, Alloc a) : vector(std::move(a)) {
        m_data() = Alloc_traits::allocate(m_allocator(), count);
        try {
            std::uninitialized_value_construct(data(), data() + count);
            m_size = count;
            m_capacity = count;
        } catch (...) {
            Alloc_traits::deallocate(m_allocator(), m_data(), count);
            throw;
        }
    }
    explicit vector(size_t count, const T& value, Alloc a) : vector(std::move(a)) {
        this->assign(count, value);
    }
    template<class It> explicit vector(It first, It last, Alloc a) : vector(std::move(a)) {
        this->assign(first, last);
    }

    vector(const vector& rhs) : vector(rhs, Alloc_traits::select_on_container_copy_construction(rhs.m_allocator())) {}

    vector(const vector& rhs, Alloc a) : vector(std::move(a)) {
        this->assign(rhs.begin(), rhs.end());
    }

    vector(vector&& rhs) noexcept : vector(std::move(rhs.m_allocator())) {
        this->adopt_allocations_of(rhs);
    }

    vector(vector&& rhs, Alloc a) noexcept : vector(std::move(a)) {
        if (this->m_allocator() == rhs.m_allocator()) {
            // we can adopt the new allocator and its memory wholesale
            this->adopt_allocations_of(rhs);
        } else {
            // we were given a different allocator and thus cannot adopt this memory
            this->assign(rhs.begin(), rhs.end());
            rhs.clear();
        }
    }

    vector& operator=(const vector& rhs) {
        if (this != &rhs) {
            // assume there is no need to propagate the allocator
            this->assign(rhs.begin(), rhs.end());
        }
        return *this;
    }

    vector& operator=(vector&& rhs) noexcept {
        if (true) {
            // assume we can adopt the new allocator and its memory wholesale
            this->clear();
            this->m_allocator() = std::move(rhs.m_allocator());
            this->adopt_allocations_of(rhs);
        }
        return *this;
    }

    void swap(vector& rhs) noexcept {
        if (true) {
            // assume we may propagate the allocator
            using std::swap;
            swap(this->m_data_and_allocator, rhs.m_data_and_allocator);
            swap(this->m_size, rhs.m_size);
            swap(this->m_capacity, rhs.m_capacity);
        }
    }

    ~vector() {
        clear();
    }

    static void relocate(T *src, size_t n, T *dest, std::true_type) {
        memcpy(dest, src, n * sizeof(T));
    }
    static void relocate(T *src, size_t n, T *dest, std::false_type) {
        std::uninitialized_move(src, src + n, dest);  // assume it's noexcept
        std::destroy(src, src + n);
    }

    void reserve(size_t cap) {
        assert(cap <= max_size());
        if (cap > capacity()) {
            pointer new_data = Alloc_traits::allocate(m_allocator(), cap);
            if (m_data()) {
                T *new_begin = static_cast<T *>(new_data);
                using trivial = std::integral_constant<bool, is_relocatable<T>::value>;
                relocate(m_data(), size(), new_begin, trivial{});
                Alloc_traits::deallocate(m_allocator(), m_data(), capacity());
            }
            m_data() = new_data;
            m_capacity = cap;
        }
    }

    void resize(size_t n) {
        if (n > size()) {
            reserve(n);
            std::uninitialized_value_construct(begin() + m_size, begin() + n);
        } else {
            std::destroy(begin() + n, end());
        }
        m_size = n;
    }

    void resize(size_t n, const T& value) {
        if (n > size()) {
            reserve(n);
            std::uninitialized_fill(data() + size(), data() + n, value);
        } else {
            std::destroy(begin() + n, end());
        }
        m_size = n;
    }

    template<class It>
    void assign(It first, It last) {
        this->reserve(std::distance(first, last));
        for (size_type i = 0; first != last && i != size(); ++i) {
            (*this)[i] = *first;
            ++first;
        }
        while (first != last) {
            this->emplace_back(*first);
            ++first;
        }
    }

    void push_back(const T& value) {
        emplace_back(value);
    }

    void push_back(T&& value) {
        emplace_back(std::move(value));
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        this->reserve(size() + 1);
        Alloc_traits::construct(m_allocator(), &(*this)[size()], std::forward<Args>(args)...);
        m_size += 1;
    }

    void clear() noexcept {
        // Unlike the standard clear(), this one is guaranteed to deallocate
        // the buffer, so that we can use it to implement the allocator-aware
        // special member functions.
        if (m_data()) {
            std::destroy(begin(), end());
            Alloc_traits::deallocate(m_allocator(), m_data(), capacity());
            m_data() = pointer{};
            m_size = 0;
            m_capacity = 0;
        }
    }

    constexpr T& operator[](size_t i) { return data()[i]; }
    constexpr const T& operator[](size_t i) const { return data()[i]; }

    constexpr T* data() noexcept { return static_cast<T*>(m_data()); }
    constexpr const T* data() const noexcept { return static_cast<T*>(m_data()); }

    constexpr Alloc get_allocator() const { return m_allocator(); }

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
    constexpr size_t capacity() const noexcept { return m_capacity; }
    constexpr bool empty() const noexcept { return size() == 0; }

    constexpr size_t max_size() const noexcept { return Alloc_traits::max_size(m_allocator()); }

private:
    pair<pointer, Alloc> m_data_and_allocator;
    size_t m_size = 0;
    size_t m_capacity = 0;

    void adopt_allocations_of(vector& rhs) noexcept {
        this->m_data() = std::exchange(rhs.m_data(), pointer{});
        this->m_size = std::exchange(rhs.m_size, 0);
        this->m_capacity = std::exchange(rhs.m_capacity, 0);
    }

    pointer& m_data() noexcept { return get<0>(m_data_and_allocator); }
    const pointer& m_data() const noexcept { return get<0>(m_data_and_allocator); }
    Alloc& m_allocator() noexcept { return get<1>(m_data_and_allocator); }
    const Alloc& m_allocator() const noexcept { return get<1>(m_data_and_allocator); }
};

template<class T, class A>
void swap(vector<T, A>& lhs, vector<T, A>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace scratch
