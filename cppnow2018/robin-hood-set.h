#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace scratch {

using std::allocator;
using std::allocator_traits;
using std::equal_to;
using std::hash;
using std::vector;

template<template<class> class Optional, class T, class Hash, class KeyEq, class Alloc>
class robin_hood_set;  // forward declaration

} // namespace scratch

namespace scratch::detail {

template<class base_type>
class robin_hood_set_iterator {
    // make friends for the benefit of "erase(it)"
    template<template<class> class, class, class, class, class> friend class scratch::robin_hood_set;

    static auto advance_to_next(base_type first, base_type last) {
        while (first != last && !first->holds_a_value()) {
            ++first;
        }
        return first;
    }

    explicit robin_hood_set_iterator(base_type here, base_type end) : m_base(here), m_end(end) {}

    static auto get_begin(base_type begin, base_type end) {
        return robin_hood_set_iterator(advance_to_next(begin, end), end);
    }

public:
    auto& operator*() const {
        return m_base->key();
    }

    auto operator->() const noexcept {
        return &m_base->key();
    }

    auto& operator++() {
        ++m_base;
        m_base = advance_to_next(m_base, m_end);
        return *this;
    }

    auto& operator++(int) {
        auto result = *this;
        ++*this;
        return result;
    }

    bool operator==(const robin_hood_set_iterator& rhs) const noexcept {
        return m_base == rhs.m_base;
    }

    bool operator!=(const robin_hood_set_iterator& rhs) const noexcept {
        return m_base != rhs.m_base;
    }

private:
    base_type m_base;
    base_type m_end;
};

} // namespace scratch::detail

namespace scratch {

template<template<class> class Optional, class T>
class robin_hood_element {
    Optional<Optional<T>> m_key;  // Outer level: !has_never_held_a_value. Inner level: holds_a_value.
    unsigned m_hash = 0;
public:
    void clear() { m_hash = 0; m_key.reset(); }
    void set_tombstone() { m_key.emplace(); }
    void set_value(T&& key, unsigned hash) { m_key.emplace(); m_key->emplace(std::move(key)); m_hash = hash; }
    bool has_never_held_a_value() const { return !m_key.has_value(); }
    bool holds_a_value() const { return m_key.has_value() && m_key->has_value(); }
    unsigned& hash() & { return m_hash; }
    unsigned hash() const & { return m_hash; }
    T& key() & { return **m_key; }
    const T& key() const & { return **m_key; }
};

template<template<class> class Optional, class T, class Hash = hash<T>, class KeyEq = equal_to<T>, class Alloc = allocator<T>>
class robin_hood_set {

    using elt_type = robin_hood_element<Optional, T>;
    using elts_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<elt_type>;
    using elts_vector_type = vector<elt_type, elts_allocator_type>;

    elts_vector_type m_elts;
    size_t m_size = 0;

public:
    using value_type = T;
    using size_type = size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = detail::robin_hood_set_iterator<typename elts_vector_type::const_iterator>;
    using const_iterator = iterator;

    robin_hood_set() : robin_hood_set(128, Alloc{}) {}
    explicit robin_hood_set(Alloc a) : robin_hood_set(128, std::move(a)) {}
    explicit robin_hood_set(size_t cap) : robin_hood_set(cap, Alloc{}) {}
    explicit robin_hood_set(size_t cap, Alloc a) : m_elts(cap, elts_allocator_type(a)) {}

    constexpr iterator begin() noexcept { return iterator::get_begin(m_elts.begin(), m_elts.end()); }
    constexpr const_iterator cbegin() noexcept { return const_iterator::get_begin(m_elts.begin(), m_elts.end()); }
    constexpr const_iterator begin() const noexcept { return const_iterator::get_begin(m_elts.begin(), m_elts.end()); }
    constexpr iterator end() noexcept { return iterator(m_elts.end(), m_elts.end()); }
    constexpr const_iterator cend() noexcept { return const_iterator(m_elts.end(), m_elts.end()); }
    constexpr const_iterator end() const noexcept { return const_iterator(m_elts.end(), m_elts.end()); }

    constexpr size_t size() const noexcept { return m_size; }
    constexpr size_t capacity() const noexcept { return m_elts.size(); }
    constexpr bool empty() const noexcept { return size() == 0; }

    constexpr size_t max_size() const noexcept { return m_elts.max_size(); }

    void swap(robin_hood_set& rhs) {
        m_elts.swap(rhs.m_elts);
        std::swap(m_size, rhs.m_size);
    }

    void reserve(size_t cap) {
        if (cap >= size()) {
            robin_hood_set copy(cap);
            for (size_t i = 0; i < capacity(); ++i) {
                if (m_elts[i].holds_a_value()) {
                    copy.unchecked_insert(std::move(m_elts[i].key()), m_elts[i].hash());
                }
            }
            copy.swap(*this);
        }
    }

    void clear() {
        for (size_t i = 0; i < capacity(); ++i) {
            m_elts[i].clear();
        }
        m_size = 0;
    }

    // Returns an iterator to the found element, or end().
    const_iterator find(const T& key) const {
        auto my_hash = Hash{}(key);
        auto i = my_hash % capacity();
        size_t my_poverty = 0;
        while (true) {
            if (m_elts[i].has_never_held_a_value()) {
                return this->end();
            } else if (poverty_of(i) < my_poverty) {
                // If `key` existed in the table, its insertion would have
                // displaced this comparatively richer element.
                return this->end();
            } else if (m_elts[i].hash() == my_hash && KeyEq{}(key, m_elts[i].key())) {
                return iterator_of(i);
            }
            i = next_probe(i);
            my_poverty += 1;
        }
    }

    // Returns true if the element was found and erased.
    bool erase(const T& key) {
        auto it = this->find(key);
        if (it != this->end()) {
            this->erase(it);
            return true;
        }
        return false;
    }

    void erase(iterator it) {
        const elt_type& elt = *it.m_base;
        const_cast<elt_type&>(elt).set_tombstone();
        m_size -= 1;
    }

    // Returns an iterator to the newly inserted, or found, key.
    iterator insert(T key) {
        auto it = find(key);
        if (it != end()) {
            return it;
        }
        if (size() == capacity()) {
            reserve(2 * capacity() + 1);
        }
        auto my_hash = Hash{}(key);
        return unchecked_insert(std::move(key), my_hash);
    }

private:
    std::pair<T, unsigned> exchange_of(int i, T key, unsigned hash) {
        elt_type elt(key, hash);
        m_elts[i].swap(elt);
        return { std::move(elt.key()), elt.hash() };
    }
    iterator iterator_of(size_t i) { return iterator(m_elts.begin() + i, m_elts.end()); }
    const_iterator iterator_of(size_t i) const { return const_iterator(m_elts.begin() + i, m_elts.end()); }

    size_t next_probe(size_t idx) const noexcept {
        return (idx + 1) % capacity();
    }

    size_t poverty_of(size_t i) const noexcept {
        // The number of failed probes this item must have done.
        size_t desired_position = m_elts[i].hash() % capacity();
        return (i + capacity() - desired_position) % capacity();
    }

    // Returns an iterator to the newly inserted element.
    iterator unchecked_insert(T key, unsigned my_hash) noexcept {
        m_size += 1;
        auto i = my_hash % capacity();
        size_t my_poverty = 0;
        Optional<iterator> result;
        while (true) {
            // Find a victim less poor than myself.
            if (m_elts[i].has_never_held_a_value()) {
                m_elts[i].set_value(std::move(key), my_hash);
                return result ? *result : iterator_of(i);
            } else if (poverty_of(i) < my_poverty) {
                if (m_elts[i].holds_a_value()) {
                    my_poverty = poverty_of(i);
                    std::swap(m_elts[i].key(), key);
                    std::swap(m_elts[i].hash(), my_hash);
                    if (!result) {
                        result = iterator_of(i);
                    }
                } else {
                    m_elts[i].set_value(std::move(key), my_hash);
                    return result ? *result : iterator_of(i);
                }
            }
            i = next_probe(i);
            my_poverty += 1;
        }
    }
};

template<template<class> class O, class... THKAs>
void swap(robin_hood_set<O, THKAs...>& lhs, robin_hood_set<O, THKAs...>& rhs) {
    lhs.swap(rhs);
}

} // namespace scratch
