#pragma once

#include "scratch/bits/containers/allocator.h"
#include "scratch/bits/containers/vector.h"
#include "scratch/bits/optional/optional.h"
#include "scratch/bits/traits-classes/allocator-traits.h"
#include "scratch/bits/type-traits/is-swappable.h"

#include <cstddef>
#include <utility>

namespace scratch {

template<class T, class Hash, class KeyEq, class Alloc>
class robin_hood_set;  // forward declaration

} // namespace scratch

namespace scratch::detail {

template<class base_type>
class robin_hood_set_iterator {
    // make friends for the benefit of "erase(it)"
    template<class, class, class, class> friend class scratch::robin_hood_set;

    static auto advance_to_next(base_type first, base_type last) {
        while (first != last && !first->has_value()) {
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
        return *(*m_base);
    }

    auto operator->() const noexcept {
        return &*(*m_base);
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

template<class T, class Hash, class KeyEq, class Alloc = allocator<T>>
class robin_hood_set {

    using keys_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<optional<T>>;
    using keys_vector_type = vector<optional<T>, keys_allocator_type>;
    using hashes_allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<unsigned>;
    using hashes_vector_type = vector<unsigned, hashes_allocator_type>;

    keys_vector_type m_keys;
    hashes_vector_type m_hashes;
    size_t m_size;

public:
    using value_type = T;
    using size_type = size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = detail::robin_hood_set_iterator<typename keys_vector_type::const_iterator>;
    using const_iterator = iterator;

    robin_hood_set() : robin_hood_set(128, Alloc{}) {}
    explicit robin_hood_set(Alloc a) : robin_hood_set(128, std::move(a)) {}
    explicit robin_hood_set(size_t cap) : robin_hood_set(cap, Alloc{}) {}
    explicit robin_hood_set(size_t cap, Alloc a) : m_keys(cap, keys_allocator_type(a)), m_hashes(cap, 0, hashes_allocator_type(a)), m_size(0) {}

    constexpr iterator begin() noexcept { return iterator::get_begin(m_keys.begin(), m_keys.end()); }
    constexpr const_iterator cbegin() noexcept { return const_iterator::get_begin(m_keys.begin(), m_keys.end()); }
    constexpr const_iterator begin() const noexcept { return const_iterator::get_begin(m_keys.begin(), m_keys.end()); }
    constexpr iterator end() noexcept { return iterator(m_keys.end(), m_keys.end()); }
    constexpr const_iterator cend() noexcept { return const_iterator(m_keys.end(), m_keys.end()); }
    constexpr const_iterator end() const noexcept { return const_iterator(m_keys.end(), m_keys.end()); }

    constexpr size_t size() const noexcept { return m_size; }
    constexpr size_t capacity() const noexcept { return m_keys.size(); }
    constexpr bool empty() const noexcept { return size() == 0; }

    constexpr size_t max_size() const noexcept { return m_keys.max_size(); }

    void swap(robin_hood_set& rhs) noexcept(is_nothrow_swappable_v<T>) {
        m_keys.swap(rhs.m_keys);
        m_hashes.swap(rhs.m_hashes);
        std::swap(m_size, rhs.m_size);
    }

    void reserve(size_t cap) {
        if (cap >= size()) {
            robin_hood_set copy(cap);
            for (size_t i = 0; i < capacity(); ++i) {
                if (holds_a_value(i)) {
                    copy.unchecked_insert(std::move(key_of(i)), std::move(hash_of(i)));
                }
            }
            copy.swap(*this);
        }
    }

    void clear() {
        for (size_t i = 0; i < capacity(); ++i) {
            m_keys[i].reset();
            m_hashes[i] = 0;
        }
        m_size = 0;
    }

    // Returns an iterator to the found element, or end().
    const_iterator find(const T& key) const {
        auto my_hash = (Hash{}(key) << 1) >> 1;
        auto i = my_hash % capacity();
        auto my_tweaked_hash = (my_hash << 1) | 1;
        size_t my_poverty = 0;
        while (true) {
            if (has_never_held_a_value(i)) {
                return this->end();
            } else if (poverty_of(i) < my_poverty) {
                // If `key` existed in the table, its insertion would have
                // displaced this comparatively richer element.
                return this->end();
            } else if (tweaked_hash_of(i) == my_tweaked_hash && KeyEq{}(key, key_of(i))) {
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
        size_t i = it.m_base - m_keys.begin();
        m_keys[i].reset();
        m_size -= 1;
        // Notice that we don't touch m_hashes[i].
        // This is important for find() to keep working.
    }

    // Returns an iterator to the newly inserted, or found, key.
    iterator insert(const T& key) {
        auto it = find(key);
        if (it != end()) {
            return it;
        }
        if (size() == capacity()) {
            reserve(2 * capacity() + 1);
        }
        auto my_hash = (Hash{}(key) << 1) >> 1;
        return unchecked_insert(key, my_hash);
    }

private:
    bool has_never_held_a_value(size_t i) const { return m_hashes[i] == 0; }
    bool holds_a_value(size_t i) const { return m_keys[i].has_value(); }
    unsigned tweaked_hash_of(size_t i) const { return m_hashes[i]; }
    unsigned hash_of(size_t i) const { return m_hashes[i] >> 1; }
    const T& key_of(size_t i) const { return *m_keys[i]; }
    iterator iterator_of(size_t i) { return iterator(m_keys.begin() + i, m_keys.end()); }
    const_iterator iterator_of(size_t i) const { return const_iterator(m_keys.begin() + i, m_keys.end()); }

    size_t next_probe(size_t idx) const {
        return (idx + 1) % capacity();
    }

    size_t poverty_of(size_t i) const {
        // The number of failed probes this item must have done.
        size_t desired_position = hash_of(i) % capacity();
        return (i + capacity() - desired_position) % capacity();
    }

    // Returns an iterator to the newly inserted element.
    iterator unchecked_insert(T key, unsigned my_hash) {
        m_size += 1;
        auto i = my_hash % capacity();
        auto my_tweaked_hash = (my_hash << 1) | 1;
        size_t my_poverty = 0;
        optional<iterator> result;
        while (true) {
            // Find a victim less poor than myself.
            if (has_never_held_a_value(i)) {
                m_keys[i].emplace(std::move(key));
                m_hashes[i] = my_tweaked_hash;
                return result ? *result : iterator_of(i);
            } else if (poverty_of(i) < my_poverty) {
                if (holds_a_value(i)) {
                    my_poverty = poverty_of(i);
                    using std::swap;
                    swap(*m_keys[i], key);
                    swap(m_hashes[i], my_tweaked_hash);
                    if (!result) {
                        result = iterator_of(i);
                    }
                } else {
                    m_keys[i].emplace(std::move(key));
                    m_hashes[i] = my_tweaked_hash;
                    return result ? *result : iterator_of(i);
                }
            }
            i = next_probe(i);
            my_poverty += 1;
        }
    }
};

template<class T, class H, class K, class A>
void swap(robin_hood_set<T, H, K, A>& lhs, robin_hood_set<T, H, K, A>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

} // namespace scratch
