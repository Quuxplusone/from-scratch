#pragma once

#include "scratch/bits/algorithm/advance.h"
#include "scratch/bits/algorithm/allocator-destroy.h"
#include "scratch/bits/algorithm/allocator-uninitialized.h"
#include "scratch/bits/algorithm/counted-copying-iterator.h"
#include "scratch/bits/containers/allocator.h"
#include "scratch/bits/stdexcept/stdexcept.h"
#include "scratch/bits/traits-classes/allocator-traits.h"
#include "scratch/bits/traits-classes/is-foo-iterator.h"
#include "scratch/bits/utility/compressed-pair.h"

#include <cstddef>
#include <utility>

namespace scratch {

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
            scratch::uninitialized_value_construct(data(), data() + count, m_allocator());
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
            if constexpr (!Alloc_traits::propagate_on_container_copy_assignment_v) {
                // we must not propagate the allocator
            } else if (m_allocator() != rhs.m_allocator()) {
                // we must propagate this new allocator
                this->clear();
                this->m_allocator() = rhs.m_allocator();
            } else {
                // there is no need to propagate the allocator
            }
            this->assign(rhs.begin(), rhs.end());
        }
        return *this;
    }

    vector& operator=(vector&& rhs) noexcept {
        if constexpr (Alloc_traits::propagate_on_container_move_assignment_v) {
            // we can adopt the new allocator and its memory wholesale
            this->clear();
            this->m_allocator() = std::move(rhs.m_allocator());
            this->adopt_allocations_of(rhs);
        } else if (m_allocator() == rhs.m_allocator()) {
            // since the allocator is unchanged, we can adopt the memory
            this->clear();
            this->adopt_allocations_of(rhs);
        } else {
            // we must not propagate this new allocator and thus cannot adopt its memory
            this->assign(rhs.begin(), rhs.end());
            rhs.clear();
        }
        return *this;
    }

    void swap(vector& rhs) noexcept {
        if constexpr (Alloc_traits::propagate_on_container_swap_v) {
            // we may propagate the allocator
            using std::swap;
            swap(this->m_data_and_allocator, rhs.m_data_and_allocator);
            swap(this->m_size, rhs.m_size);
            swap(this->m_capacity, rhs.m_capacity);
        } else if (m_allocator() == rhs.m_allocator()) {
            // we must not propagate the allocator, but at least we can swap
            using std::swap;
            swap(this->m_data(), rhs.m_data());
            swap(this->m_size, rhs.m_size);
            swap(this->m_capacity, rhs.m_capacity);
        } else {
            // Under the standard, this has undefined behavior.
            auto temp = std::move(*this);  // save the pointer to my data
            *this = std::move(rhs);  // copy rhs's data into my heap, and clear rhs
            rhs = std::move(temp);  // copy my data into rhs's heap
        }
    }

    ~vector() {
        clear();
    }

    void reserve(size_t cap) {
        if (cap > max_size()) {
            throw length_error(exception::nocopy, "reserve");
        } else if (cap > capacity()) {
            pointer new_data = Alloc_traits::allocate(m_allocator(), cap);
            if (m_data()) {
                T *new_begin = static_cast<T *>(new_data);
                scratch::uninitialized_move_if_noexcept(begin(), end(), new_begin, m_allocator());
                scratch::destroy(begin(), end(), m_allocator());
                Alloc_traits::deallocate(m_allocator(), m_data(), capacity());
            }
            m_data() = new_data;
            m_capacity = cap;
        }
    }

    void resize(size_t n) {
        if (n > size()) {
            reserve(n);
            scratch::uninitialized_value_construct(begin() + m_size, begin() + n, m_allocator());
        } else {
            scratch::destroy(begin() + n, end(), m_allocator());
        }
        m_size = n;
    }

    void resize(size_t n, const T& value) {
        if (n > size()) {
            reserve(n);
            scratch::uninitialized_fill(data() + size(), data() + n, value, m_allocator());
        } else {
            scratch::destroy(begin() + n, end(), m_allocator());
        }
        m_size = n;
    }

    // resize_smaller is NOT a standard library method!
    void resize_smaller(size_t n) {
        if (n > size()) throw invalid_argument(exception::nocopy, "resize_smaller");
        scratch::destroy(begin() + n, end(), m_allocator());
        m_size = n;
    }

    void assign(size_t count, const T& value) {
        using It = counted_copying_iterator<T>;
        this->assign(It(count, value), It());
    }

    template<class It>
    void assign(It first, It last) {
        if constexpr (is_forward_iterator_v<It>) {
            this->reserve(scratch::distance(first, last));
        }
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
            scratch::destroy(begin(), end(), m_allocator());
            Alloc_traits::deallocate(m_allocator(), m_data(), capacity());
            m_data() = pointer{};
            m_size = 0;
            m_capacity = 0;
        }
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
    compressed_pair<pointer, Alloc> m_data_and_allocator;
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
