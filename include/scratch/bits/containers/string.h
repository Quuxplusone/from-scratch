#pragma once

#include "scratch/bits/algorithm/advance.h"
#include "scratch/bits/algorithm/copy.h"
#include "scratch/bits/algorithm/counted-copying-iterator.h"
#include "scratch/bits/containers/allocator.h"
#include "scratch/bits/stdexcept/stdexcept.h"
#include "scratch/bits/traits-classes/allocator-traits.h"
#include "scratch/bits/traits-classes/is-foo-iterator.h"
#include "scratch/bits/type-traits/is-fooible.h"
#include "scratch/bits/utility/compressed-pair.h"

#include <cstddef>
#include <utility>

namespace scratch::detail {

template<class T>
inline T *empty_string() {
    static T instance[1];
    return instance;
}

} // namespace scratch::detail

namespace scratch {

template<class T, class Alloc = allocator<T>>
class basic_string {
    static_assert(is_pod_v<T>, "Non-POD types are not allowed as character types");
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

    static constexpr size_type npos = size_type(-1);

    basic_string() noexcept : basic_string(Alloc{}) {}
    explicit basic_string(size_t count) : basic_string(count, Alloc{}) {}
    explicit basic_string(size_t count, const T& value) : basic_string(count, value, Alloc{}) {}
             basic_string(const T *s) : basic_string(s, Alloc{}) {}
    explicit basic_string(const basic_string& s, size_type pos) : basic_string(s, pos, npos, Alloc{}) {}
    explicit basic_string(const basic_string& s, size_type pos, size_type n) : basic_string(s, pos, n, Alloc{}) {}
    template<class It> explicit basic_string(It first, It last) : basic_string(first, last, Alloc{}) {}

    explicit basic_string(Alloc a) noexcept : m_data_and_allocator(pointer{}, std::move(a)) {}
    explicit basic_string(size_t count, Alloc a) : basic_string(std::move(a)) {
        if (count > max_size()) throw length_error(exception::nocopy, "basic_string constructor");
        m_data() = Alloc_traits::allocate(m_allocator(), count + 1);
        for (size_t i=0; i < count + 1; ++i) {
            data()[i] = T();
        }
        m_size = count;
        m_capacity = count;
    }
    explicit basic_string(size_t count, const T& value, Alloc a) : basic_string(std::move(a)) {
        this->assign(count, value);
    }
    explicit basic_string(const T *s, Alloc a) : basic_string(std::move(a)) {
        this->assign(s, s + strlen(s));
    }
    explicit basic_string(const basic_string& s, size_type pos, Alloc a) : basic_string(s, pos, npos, std::move(a)) {}
    explicit basic_string(const basic_string& s, size_type pos, size_type n, Alloc a) : basic_string(std::move(a)) {
        if (pos > s.size()) throw out_of_range(exception::nocopy, "basic_string constructor");
        if (n == npos || n > s.size() - pos) n = (s.size() - pos);
        this->assign(&s[pos], &s[pos] + n);
    }

    template<class It> explicit basic_string(It first, It last, Alloc a) : basic_string(std::move(a)) {
        this->assign(first, last);
    }

    basic_string(const basic_string& rhs) : basic_string(rhs, Alloc_traits::select_on_container_copy_construction(rhs.m_allocator())) {}

    basic_string(const basic_string& rhs, Alloc a) : basic_string(std::move(a)) {
        this->assign(rhs.begin(), rhs.end());
    }

    basic_string(basic_string&& rhs) noexcept : basic_string(std::move(rhs.m_allocator())) {
        this->adopt_allocations_of(rhs);
    }

    basic_string(basic_string&& rhs, Alloc a) noexcept : basic_string(std::move(a)) {
        if (this->m_allocator() == rhs.m_allocator()) {
            // we can adopt the new allocator and its memory wholesale
            this->adopt_allocations_of(rhs);
        } else {
            // we were given a different allocator and thus cannot adopt this memory
            this->assign(rhs.begin(), rhs.end());
            rhs.clear();
        }
    }

    basic_string& operator=(const basic_string& rhs) {
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

    basic_string& operator=(basic_string&& rhs) noexcept {
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

    void swap(basic_string& rhs) noexcept {
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

    ~basic_string() {
        clear();
    }

    void reserve(size_t cap) {
        if (cap > max_size()) {
            throw length_error(exception::nocopy, "reserve");
        } else if (cap > capacity()) {
            pointer new_data = Alloc_traits::allocate(m_allocator(), cap + 1);
            if (m_data()) {
                T *new_begin = static_cast<T *>(new_data);
                scratch::copy(begin(), end() + 1, new_begin);
                Alloc_traits::deallocate(m_allocator(), m_data(), capacity() + 1);
            }
            using std::swap;
            swap(m_data(), new_data);
            m_capacity = cap;
        }
    }

    void resize(size_t n) {
        resize(n, T());
    }

    void resize(size_t n, const T& value) {
        if (n > size()) {
            reserve(n);
            scratch::fill(begin() + m_size, begin() + n, value);
        }
        data()[n] = T();
        m_size = n;
    }

    // resize_smaller is NOT a standard library method!
    void resize_smaller(size_t n) {
        if (n > size()) throw invalid_argument(exception::nocopy, "resize_smaller");
        data()[n] = T();
        m_size = n;
    }

    auto& assign(size_t count, const T& value) {
        using It = counted_copying_iterator<T>;
        return this->assign(It(count, value), It());
    }

    auto& assign(const basic_string& s) { return assign(s.begin(), s.end()); }
    auto& assign(const T *s) { return assign(s, strlen(s)); }
    auto& assign(const T *s, size_type n) { return assign(s, s + n); }

    template<class It>
    auto& assign(It first, It last) {
        if constexpr (is_forward_iterator_v<It>) {
            this->reserve(scratch::distance(first, last));
        }
        for (size_type i = 0; first != last && i != size(); ++i) {
            (*this)[i] = *first;
            ++first;
        }
        while (first != last) {
            this->push_back(*first);
            ++first;
        }
        return *this;
    }

    auto& append(size_t count, const T& value) {
        using It = counted_copying_iterator<T>;
        return append(It(count, value), It());
    }

    auto& append(const basic_string& s) { return append(s.begin(), s.end()); }
    auto& append(const T *s) { return append(s, s + strlen(s)); }
    auto& append(const T *s, size_type n) { return append(s, s + n); }

    template<class It>
    auto& append(It first, It last) {
        if constexpr (is_forward_iterator_v<It>) {
            this->reserve(capacity() + scratch::distance(first, last));
            while (first != last) {
                this->push_back(*first);
                ++first;
            }
        } else {
            // Preserve the strong exception guarantee.
            basic_string temp(first, last, m_allocator());
            append(temp);
        }
        return *this;
    }

    void push_back(T value) {
        this->reserve(size() + 1);
        data()[size()] = value;
        data()[size() + 1] = T();  // null-terminate
        m_size += 1;
    }

    auto& operator=(const T *s) { return assign(s); }
    auto& operator=(T ch) { return assign(1, ch); }

    auto& operator+=(const basic_string& s) { return append(s); }
    auto& operator+=(const T *s) { return append(s); }
    auto& operator+=(T ch) { return append(1, ch); }

    void clear() noexcept {
        // Unlike the standard clear(), this one is guaranteed to deallocate
        // the buffer, so that we can use it to implement the allocator-aware
        // special member functions.
        if (m_data()) {
            Alloc_traits::deallocate(m_allocator(), m_data(), capacity() + 1);
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

    constexpr T* data() noexcept { return m_data() ? static_cast<T*>(m_data()) : detail::empty_string<T>(); }
    constexpr const T* data() const noexcept { return m_data() ? static_cast<T*>(m_data()) : detail::empty_string<T>(); }
    constexpr const T* c_str() const noexcept { return m_data() ? static_cast<T*>(m_data()) : detail::empty_string<T>(); }

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
    constexpr size_t length() const noexcept { return m_size; }
    constexpr size_t capacity() const noexcept { return m_capacity; }
    constexpr bool empty() const noexcept { return size() == 0; }

    constexpr size_t max_size() const noexcept { return Alloc_traits::max_size(m_allocator()) - 1; }

private:
    compressed_pair<pointer, Alloc> m_data_and_allocator;
    size_t m_size = 0;
    size_t m_capacity = 0;

    void adopt_allocations_of(basic_string& rhs) noexcept {
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
void swap(basic_string<T, A>& lhs, basic_string<T, A>& rhs) noexcept {
    lhs.swap(rhs);
}

template<class T, class A>
basic_string<T,A> operator+ (basic_string<T,A> lhs, const basic_string<T,A>& rhs) {
    lhs += rhs;
    return lhs;
}
template<class T, class A>
basic_string<T,A> operator+ (basic_string<T,A> lhs, const T *rhs) {
    lhs += rhs;
    return lhs;
}
template<class T, class A>
basic_string<T,A> operator+ (const T *lhs, const basic_string<T,A>& rhs) {
    basic_string<T,A> lhs_str(lhs);
    lhs_str += rhs;
    return lhs_str;
}

using string = basic_string<char>;

} // namespace scratch
