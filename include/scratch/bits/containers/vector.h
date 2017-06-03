#pragma once

#include "scratch/bits/algorithm/advance.h"
#include "scratch/bits/algorithm/allocator-destroy.h"
#include "scratch/bits/algorithm/allocator-uninitialized.h"
#include "scratch/bits/containers/allocator.h"
#include "scratch/bits/stdexcept/stdexcept.h"
#include "scratch/bits/traits-classes/allocator-traits.h"
#include "scratch/bits/traits-classes/is-foo-iterator.h"
#include "scratch/bits/type-traits/is-swappable.h"
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
    using size_type = size_t;
    using difference_type = ptrdiff_t;
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

    explicit vector(Alloc a) noexcept : m_data_and_allocator(pointer{}, std::move(a)) {
        set_size_and_capacity(0);
    }
    explicit vector(size_t count, Alloc a) : vector(std::move(a)) {
        m_data() = Alloc_traits::allocate(m_allocator(), count * sizeof (T));
        scratch::uninitialized_value_construct(data(), data() + count, m_allocator());
        set_size_and_capacity(count);
    }
    explicit vector(size_t count, const T& value, Alloc a) : vector(std::move(a)) {
        m_data() = Alloc_traits::allocate(m_allocator(), count * sizeof (T));
        scratch::uninitialized_fill(data(), data() + count, value, m_allocator());
        set_size_and_capacity(count);
    }
    template<class It> explicit vector(It first, It last, Alloc a) : vector(std::move(a)) {
        this->assign(first, last);
    }

    vector(const vector& rhs) : vector(Alloc_traits::select_on_container_copy_construction(rhs.m_allocator())) {
        this->assign(rhs.begin(), rhs.end());
    }

    vector(const vector& rhs, Alloc a) : vector(std::move(a)) {
        this->assign(rhs.begin(), rhs.end());
    }

    vector(vector&& rhs) noexcept : vector(std::move(rhs.m_allocator())) {
        this->m_data() = std::exchange(rhs.m_data(), pointer{});
        this->m_size = std::exchange(rhs.m_size, 0);
        this->m_capacity = std::exchange(rhs.m_capacity, 0);
    }

    vector(vector&& rhs, Alloc a) noexcept : vector(std::move(a)) {
        this->m_data() = std::exchange(rhs.m_data(), pointer{});
        this->m_size = std::exchange(rhs.m_size, 0);
        this->m_capacity = std::exchange(rhs.m_capacity, 0);
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
            this->m_data() = std::exchange(rhs.m_data(), pointer{});
            this->m_size = std::exchange(rhs.m_size, 0);
            this->m_capacity = std::exchange(rhs.m_capacity, 0);
        } else if (m_allocator() == rhs.m_allocator()) {
            // since the allocator is unchanged, we can adopt the memory
            this->clear();
            this->m_data() = std::exchange(rhs.m_data(), pointer{});
            this->m_size = std::exchange(rhs.m_size, 0);
            this->m_capacity = std::exchange(rhs.m_capacity, 0);
        } else {
            // we must not propagate this new allocator and thus cannot adopt its memory
            this->assign(rhs.begin(), rhs.end());
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
        }
    }

    ~vector() {
        if (m_data()) {
            scratch::destroy(begin(), end(), m_allocator());
            Alloc_traits::deallocate(m_allocator(), m_data(), capacity() * sizeof (T));
        }
    }

    void reserve(size_t cap) {
        if (cap > max_size()) {
            throw length_error(exception::nocopy, "reserve");
        } else if (cap > capacity()) {
            pointer new_data = Alloc_traits::allocate(m_allocator(), cap * sizeof (T));
            if (m_data()) {
                T *new_begin = static_cast<T *>(new_data);
                scratch::uninitialized_move_if_noexcept(begin(), end(), new_begin, m_allocator());
                scratch::destroy(begin(), end(), m_allocator());
                Alloc_traits::deallocate(m_allocator(), m_data(), capacity() * sizeof (T));
            }
            using std::swap;
            swap(m_data(), new_data);
            m_capacity = cap;
        }
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
        scratch::destroy(begin(), end(), m_allocator());
        m_size = 0;
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

    constexpr size_t max_size() const noexcept { return size_t(-1) / sizeof (T); }

private:
    compressed_pair<pointer, Alloc> m_data_and_allocator;
    size_t m_size;
    size_t m_capacity;

    pointer& m_data() noexcept { return get<0>(m_data_and_allocator); }
    const pointer& m_data() const noexcept { return get<0>(m_data_and_allocator); }
    Alloc& m_allocator() noexcept { return get<1>(m_data_and_allocator); }
    const Alloc& m_allocator() const noexcept { return get<1>(m_data_and_allocator); }

    void set_size_and_capacity(size_t n) noexcept {
        m_size = n;
        m_capacity = n;
    }
};

template<class T>
void swap(vector<T>& lhs, vector<T>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace scratch
