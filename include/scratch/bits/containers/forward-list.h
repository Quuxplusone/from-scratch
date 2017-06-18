#pragma once

#include "scratch/bits/algorithm/iterator-tags.h"
#include "scratch/bits/algorithm/counted-copying-iterator.h"
#include "scratch/bits/containers/allocator.h"
#include "scratch/bits/traits-classes/allocator-traits.h"
#include "scratch/bits/traits-classes/is-foo-iterator.h"
#include "scratch/bits/type-traits/conditional.h"
#include "scratch/bits/utility/compressed-pair.h"

#include <cstddef>
#include <utility>

namespace scratch {
    template<typename T> class forward_list;  // forward declaration
} // namespace scratch

namespace scratch::detail {

template<typename T> struct forward_list_node;  // forward declaration

struct forward_list_node_base {
    forward_list_node_base *next;

    explicit forward_list_node_base(forward_list_node_base *n) : next(n) {}

    template<typename T>
    forward_list_node<T> *m_next() const { return static_cast<forward_list_node<T>*>(this->next); }
};

template<typename T>
struct forward_list_node : compressed_element<T, 0>, forward_list_node_base
{
    template<typename... Args>
    explicit forward_list_node(forward_list_node_base *n, Args&&... args) :
        compressed_element<T, 0>(std::forward<Args>(args)...),
        forward_list_node_base(n)
    {}

    T& m_data() { return this->get_element(index_constant<0>{}); }
    const T& m_data() const { return this->get_element(index_constant<0>{}); }
};

template<typename T, bool IsConst>
class forward_list_iterator
{
    using node_pointer = forward_list_node<T>*;
public:
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = conditional_t<IsConst, const value_type, value_type>*;
    using reference = conditional_t<IsConst, const value_type, value_type>&;
    using iterator_category = forward_iterator_tag;

    forward_list_iterator() = default;

    auto& operator++() { m_ptr = m_ptr->template m_next<T>(); return *this; }
    auto operator++(int) { auto old = *this; ++*this; return old; }

    reference operator*() const { return static_cast<node_pointer>(m_ptr)->m_data(); }
    pointer operator->() const { return &static_cast<node_pointer>(m_ptr)->m_data(); }

    operator forward_list_iterator<T, true>() const noexcept { return forward_list_iterator<T, true>(m_ptr); }

    template<bool C> bool operator==(const forward_list_iterator<T,C>& rhs) const noexcept { return m_ptr == rhs.m_ptr; }
    template<bool C> bool operator!=(const forward_list_iterator<T,C>& rhs) const noexcept { return m_ptr != rhs.m_ptr; }
private:
    friend class forward_list<T>;
    friend class forward_list_iterator<T, !IsConst>;

    forward_list_iterator(forward_list_node_base *p) : m_ptr(p) {}
    node_pointer get_node_pointer() { return static_cast<node_pointer>(m_ptr); }
    operator forward_list_iterator<T, false>() const noexcept { return forward_list_iterator<T, false>(m_ptr); }

    forward_list_node_base *m_ptr = nullptr;
};

} // namespace scratch::detail

namespace scratch {

template<class T>
class forward_list {
    using Alloc = allocator<T>;
    using Alloc_traits = allocator_traits<allocator<T>>;
    using node_type = detail::forward_list_node<T>;
    using node_pointer = node_type*;
    using NodeAlloc = typename Alloc_traits::template rebind_alloc<node_type>;
    using NodeAlloc_traits = allocator_traits<NodeAlloc>;
public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = typename Alloc_traits::pointer;
    using const_pointer = typename Alloc_traits::const_pointer;
    using iterator = detail::forward_list_iterator<T, false>;
    using const_iterator = detail::forward_list_iterator<T, true>;

    forward_list() noexcept : forward_list(Alloc{}) {}
    explicit forward_list(size_t count) : forward_list(count, Alloc{}) {}
    explicit forward_list(size_t count, const T& value) : forward_list(count, value, Alloc{}) {}

    template<class It, class = enable_if_t<is_input_iterator_v<It>>>
    explicit forward_list(It first, It last) : forward_list(first, last, Alloc{}) {}

    explicit forward_list(Alloc a) noexcept : m_head_and_allocator(node_pointer{}, std::move(a)) {}
    explicit forward_list(size_t count, Alloc a) : forward_list(std::move(a)) {
        iterator previous = this->before_begin();
        for (size_t i = 0; i < count; ++i) {
            previous = this->emplace_after(previous);
        }
    }
    explicit forward_list(size_t count, const T& value, Alloc a) : forward_list(std::move(a)) {
        this->assign(count, value);
    }
    template<class It, class = enable_if_t<is_input_iterator_v<It>>>
    explicit forward_list(It first, It last, Alloc a) : forward_list(std::move(a)) {
        this->assign(first, last);
    }

    forward_list(const forward_list& rhs) : forward_list(Alloc_traits::select_on_container_copy_construction(rhs.m_allocator())) {
        this->assign(rhs.begin(), rhs.end());
    }

    forward_list(const forward_list& rhs, Alloc a) : forward_list(std::move(a)) {
        this->assign(rhs.begin(), rhs.end());
    }

    forward_list(forward_list&& rhs) noexcept : forward_list(std::move(rhs.m_allocator())) {
        this->m_before_head().next = std::exchange(rhs.m_before_head().next, {});
        this->m_size = std::exchange(rhs.m_size, 0);
    }

    forward_list(forward_list&& rhs, Alloc a) noexcept : forward_list(std::move(a)) {
        this->m_before_head().next = std::exchange(rhs.m_before_head().next, {});
        this->m_size = std::exchange(rhs.m_size, 0);
    }

    forward_list& operator=(const forward_list& rhs) {
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

    forward_list& operator=(forward_list&& rhs) noexcept {
        if constexpr (Alloc_traits::propagate_on_container_move_assignment_v) {
            // we can adopt the new allocator and its memory wholesale
            this->clear();
            this->m_allocator() = std::move(rhs.m_allocator());
            this->m_before_head().next = std::exchange(rhs.m_before_head().next, {});
            this->m_size = std::exchange(rhs.m_size, 0);
        } else if (m_allocator() == rhs.m_allocator()) {
            // since the allocator is unchanged, we can adopt the memory
            this->clear();
            this->m_before_head().next = std::exchange(rhs.m_before_head().next, {});
            this->m_size = std::exchange(rhs.m_size, 0);
        } else {
            // we must not propagate this new allocator and thus cannot adopt its memory
            this->assign(rhs.begin(), rhs.end());
        }
        return *this;
    }

    void swap(forward_list& rhs) noexcept {
        if constexpr (Alloc_traits::propagate_on_container_swap_v) {
            // we may propagate the allocator
            using std::swap;
            swap(this->m_head_and_allocator, rhs.m_head_and_allocator);
            swap(this->m_size, rhs.m_size);
        } else if (m_allocator() == rhs.m_allocator()) {
            // we must not propagate the allocator, but at least we can swap
            using std::swap;
            swap(this->m_before_head(), rhs.m_before_head());
            swap(this->m_size, rhs.m_size);
        } else {
            // Under the standard, this has undefined behavior.
        }
    }

    ~forward_list() {
        clear();
    }

    void assign(size_t count, const T& value) {
        using It = counted_copying_iterator<T>;
        this->assign(It(count, value), It());
    }

    template<class It, class = enable_if_t<is_input_iterator_v<It>>>
    void assign(It first, It last) {
        iterator previous = this->before_begin();
        iterator it = this->begin();
        while (first != last && it != this->end()) {
            *it = *first;
            previous = it;
            ++first;
            ++it;
        }
        if (it != this->end()) {
            this->erase_after(previous, this->end());
        }
        while (first != last) {
            previous = this->emplace_after(previous, *first);
            ++first;
        }
    }

    template<typename... Args>
    void emplace_front(Args&&... args) {
        emplace_after(before_begin(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    iterator emplace_after(const_iterator it, Args&&... args) {
        node_pointer prev = it.get_node_pointer();
        node_pointer next = prev->template m_next<T>();

        NodeAlloc a(m_allocator());
        node_pointer p = NodeAlloc_traits::allocate(a, 1);
        try {
            NodeAlloc_traits::construct(a, p, next, std::forward<Args>(args)...);
            m_size += 1;
        } catch (...) {
            NodeAlloc_traits::deallocate(a, p, 1);
            throw;
        }
        prev->next = p;
        return iterator(p);
    }

    iterator erase_after(const_iterator it) {
        node_pointer prev = it.get_node_pointer();
        node_pointer next = prev->template m_next<T>();

        NodeAlloc a(m_allocator());
        prev->next = next->next;
        NodeAlloc_traits::destroy(a, next);
        NodeAlloc_traits::deallocate(a, next, 1);
        m_size -= 1;
        return iterator(prev->next);
    }

    iterator erase_after(const_iterator first, const_iterator last) {
        const_iterator next = first;
        ++next;
        while (next != last) {
            next = this->erase_after(first);
        }
        return last;
    }

    void clear() noexcept {
        erase_after(before_begin(), end());
    }

    constexpr Alloc get_allocator() const { return m_allocator(); }

    constexpr T& front() { return *begin(); }
    constexpr const T& front() const { return *begin(); }

    constexpr iterator before_begin() noexcept { return &m_before_head(); }
    constexpr const_iterator cbefore_begin() noexcept { return &m_before_head(); }
    constexpr const_iterator before_begin() const noexcept { return &m_before_head(); }

    constexpr iterator begin() noexcept { return m_before_head().template m_next<T>(); }
    constexpr const_iterator cbegin() noexcept { return m_before_head().template m_next<T>(); }
    constexpr const_iterator begin() const noexcept { return m_before_head().template m_next<T>(); }
    constexpr iterator end() noexcept { return nullptr; }
    constexpr const_iterator cend() noexcept { return nullptr; }
    constexpr const_iterator end() const noexcept { return nullptr; }

    constexpr size_t size() const noexcept { return m_size; }
    constexpr bool empty() const noexcept { return size() == 0; }

private:
    compressed_pair<detail::forward_list_node_base, Alloc> m_head_and_allocator;
    size_t m_size = 0;

    detail::forward_list_node_base& m_before_head() noexcept { return get<0>(m_head_and_allocator); }
    const detail::forward_list_node_base& m_before_head() const noexcept { return get<0>(m_head_and_allocator); }
    Alloc& m_allocator() noexcept { return get<1>(m_head_and_allocator); }
    const Alloc& m_allocator() const noexcept { return get<1>(m_head_and_allocator); }
    node_pointer m_head() const noexcept { return static_cast<node_pointer>(m_before_head()->next); }
};

template<class T>
void swap(forward_list<T>& lhs, forward_list<T>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace scratch
