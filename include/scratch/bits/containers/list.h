#pragma once

#include "scratch/bits/algorithm/advance.h"
#include "scratch/bits/algorithm/iterator-tags.h"
#include "scratch/bits/algorithm/counted-copying-iterator.h"
#include "scratch/bits/containers/allocator.h"
#include "scratch/bits/traits-classes/allocator-traits.h"
#include "scratch/bits/traits-classes/pointer-traits.h"
#include "scratch/bits/traits-classes/is-foo-iterator.h"
#include "scratch/bits/type-traits/conditional.h"
#include "scratch/bits/utility/compressed-element.h"
#include "scratch/bits/utility/compressed-pair.h"

#include <cstddef>
#include <utility>

namespace scratch {
    template<typename T, typename A = allocator<T>> class list;  // forward declaration
} // namespace scratch

namespace scratch::detail {

template<typename VoidPtr> struct list_node_base;  // forward declaration
template<typename VoidPtr, typename T> struct list_node;  // forward declaration

template<class VoidPtr>
using list_node_base_ptr = typename pointer_traits<VoidPtr>::template rebind<list_node_base<VoidPtr>>;

template<class VoidPtr, class T>
using list_node_ptr = typename pointer_traits<VoidPtr>::template rebind<list_node<VoidPtr, T>>;

template<class VoidPtr>
struct list_node_base {
    list_node_base_ptr<VoidPtr> prev;
    list_node_base_ptr<VoidPtr> next;

    explicit list_node_base(list_node_base_ptr<VoidPtr> p, list_node_base_ptr<VoidPtr> n) : prev(p), next(n) {}

    template<typename T>
    auto m_fancy_prev() const { return static_cast<list_node_ptr<VoidPtr, T>>(this->prev); }

    template<typename T>
    auto m_fancy_next() const { return static_cast<list_node_ptr<VoidPtr, T>>(this->next); }

    template<typename T>
    auto m_plain_prev() const { return static_cast<list_node<VoidPtr, T>*>(this->m_fancy_prev<T>()); }

    template<typename T>
    auto m_plain_next() const { return static_cast<list_node<VoidPtr, T>*>(this->m_fancy_next<T>()); }
};

template<class VoidPtr, class T>
struct list_node : compressed_element<T, 0>, list_node_base<VoidPtr>
{
    template<typename... Args>
    explicit list_node(list_node_base_ptr<VoidPtr> p, list_node_base_ptr<VoidPtr> n, Args&&... args) :
        compressed_element<T, 0>(std::forward<Args>(args)...),
        list_node_base<VoidPtr>(p, n)
    {}

    T& m_data() { return this->get_element(index_constant<0>{}); }
    const T& m_data() const { return this->get_element(index_constant<0>{}); }
};

template<typename VoidPtr, typename T, bool IsConst>
class list_iterator
{
    using node_type = list_node<VoidPtr, T>;
public:
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = conditional_t<IsConst, const value_type, value_type>*;
    using reference = conditional_t<IsConst, const value_type, value_type>&;
    using iterator_category = bidirectional_iterator_tag;

    list_iterator() = default;

    auto& operator++() { m_ptr = m_ptr->template m_plain_next<T>(); return *this; }
    auto operator++(int) { auto old = *this; ++*this; return old; }

    auto& operator--() { m_ptr = m_ptr->template m_plain_prev<T>(); return *this; }
    auto operator--(int) { auto old = *this; --*this; return old; }

    reference operator*() const { return static_cast<node_type *>(m_ptr)->m_data(); }
    pointer operator->() const { return &static_cast<node_type *>(m_ptr)->m_data(); }

    operator list_iterator<VoidPtr, T, true>() const noexcept { return list_iterator<VoidPtr, T, true>(m_ptr); }

    template<bool C> bool operator==(const list_iterator<VoidPtr,T,C>& rhs) const noexcept { return m_ptr == rhs.m_ptr; }
    template<bool C> bool operator!=(const list_iterator<VoidPtr,T,C>& rhs) const noexcept { return m_ptr != rhs.m_ptr; }
private:
    template<class U, class Alloc> friend class scratch::list;
    friend class list_iterator<VoidPtr, T, !IsConst>;

    list_iterator(const list_node_base<VoidPtr> *p) : m_ptr(const_cast<list_node_base<VoidPtr> *>(p)) {}
    node_type *get_node_pointer() { return static_cast<node_type *>(m_ptr); }
    operator list_iterator<VoidPtr, T, false>() const noexcept { return list_iterator<VoidPtr, T, false>(m_ptr); }

    list_node_base<VoidPtr> *m_ptr = nullptr;
};

} // namespace scratch::detail

namespace scratch {

template<class T, class Alloc>
class list {
    using Alloc_traits = allocator_traits<Alloc>;
    using VoidPtr = typename Alloc_traits::void_pointer;
    using node_base_pointer = detail::list_node_base_ptr<VoidPtr>;
    using node_type = detail::list_node<VoidPtr, T>;
    using node_pointer = detail::list_node_ptr<VoidPtr, T>;
    using NodeAlloc = typename Alloc_traits::template rebind_alloc<node_type>;
    using NodeAlloc_traits = allocator_traits<NodeAlloc>;

    static_assert(is_same_v<node_pointer, typename NodeAlloc_traits::pointer>);

public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = typename Alloc_traits::pointer;
    using const_pointer = typename Alloc_traits::const_pointer;
    using iterator = detail::list_iterator<VoidPtr, T, false>;
    using const_iterator = detail::list_iterator<VoidPtr, T, true>;

    list() noexcept : list(Alloc{}) {}
    explicit list(size_t count) : list(count, Alloc{}) {}
    explicit list(size_t count, const T& value) : list(count, value, Alloc{}) {}

    template<class It, class = enable_if_t<is_input_iterator_v<It>>>
    explicit list(It first, It last) : list(first, last, Alloc{}) {}

    explicit list(Alloc a) noexcept :
        m_head_and_allocator(
            detail::list_node_base<VoidPtr>{node_pointer{}, node_pointer{}},
            std::move(a)
        )
    {
        this->fix_up_internal_pointers();
    }

    explicit list(size_t count, Alloc a) : list(std::move(a)) {
        for (size_t i = 0; i < count; ++i) {
            this->emplace_back();
        }
    }
    explicit list(size_t count, const T& value, Alloc a) : list(std::move(a)) {
        this->assign(count, value);
    }
    template<class It, class = enable_if_t<is_input_iterator_v<It>>>
    explicit list(It first, It last, Alloc a) : list(std::move(a)) {
        this->assign(first, last);
    }

    list(const list& rhs) : list(rhs, Alloc_traits::select_on_container_copy_construction(rhs.m_allocator())) {}

    list(const list& rhs, Alloc a) : list(std::move(a)) {
        this->assign(rhs.begin(), rhs.end());
    }

    list(list&& rhs) noexcept : list(std::move(rhs.m_allocator())) {
        // we can adopt the new allocator and its memory wholesale
        this->adopt_allocations_of(rhs);
    }

    list(list&& rhs, Alloc a) noexcept : list(std::move(a)) {
        if (this->m_allocator() == rhs.m_allocator()) {
            // we can adopt the new allocator and its memory wholesale
            this->adopt_allocations_of(rhs);
        } else {
            // we were given a different allocator and thus cannot adopt this memory
            this->assign(rhs.begin(), rhs.end());
            rhs.clear();
        }
    }

    list& operator=(const list& rhs) {
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

    list& operator=(list&& rhs) noexcept {
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

    void swap(list& rhs) noexcept {
        if constexpr (Alloc_traits::propagate_on_container_swap_v) {
            // we may propagate the allocator
            using std::swap;
            swap(this->m_head_and_allocator, rhs.m_head_and_allocator);
            swap(this->m_size, rhs.m_size);
            this->fix_up_internal_pointers();
            rhs.fix_up_internal_pointers();
        } else if (m_allocator() == rhs.m_allocator()) {
            // we must not propagate the allocator, but at least we can swap
            using std::swap;
            swap(this->m_after_tail(), rhs.m_after_tail());
            swap(this->m_size, rhs.m_size);
            this->fix_up_internal_pointers();
            rhs.fix_up_internal_pointers();
        } else {
            // Under the standard, this has undefined behavior.
            auto temp = std::move(*this);  // save the pointer to my data
            *this = std::move(rhs);  // copy rhs's data into my heap, and clear rhs
            rhs = std::move(temp);  // copy my data into rhs's heap
        }
    }

    ~list() {
        clear();
    }

    void assign(size_t count, const T& value) {
        using It = counted_copying_iterator<T>;
        this->assign(It(count, value), It());
    }

    template<class It, class = enable_if_t<is_input_iterator_v<It>>>
    void assign(It first, It last) {
        iterator it = this->begin();
        while (first != last && it != this->end()) {
            *it = *first;
            ++first;
            ++it;
        }
        this->erase(it, this->end());
        while (first != last) {
            this->emplace_back(*first);
            ++first;
        }
    }

    template<typename... Args>
    void emplace_front(Args&&... args) {
        emplace(begin(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        emplace(end(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    iterator emplace(const_iterator it, Args&&... args) {
        node_type *raw_prev = it.get_node_pointer();
        node_pointer next = raw_prev->template m_fancy_next<T>();
        node_pointer prev = next->template m_fancy_prev<T>();

        NodeAlloc a(m_allocator());
        node_pointer p = NodeAlloc_traits::allocate(a, 1);
        try {
            NodeAlloc_traits::construct(a, static_cast<node_type*>(p), prev, next, std::forward<Args>(args)...);
            m_size += 1;
        } catch (...) {
            NodeAlloc_traits::deallocate(a, p, 1);
            throw;
        }
        prev->next = p;
        next->prev = p;
        return iterator(static_cast<node_type*>(p));
    }

    iterator erase(const_iterator it) {
        node_type *raw_current = it.get_node_pointer();
        node_pointer prev = raw_current->template m_fancy_prev<T>();
        node_pointer next = raw_current->template m_fancy_next<T>();
        node_pointer current = prev->template m_fancy_next<T>();

        NodeAlloc a(m_allocator());
        prev->next = next;
        next->prev = prev;
        NodeAlloc_traits::destroy(a, raw_current);
        NodeAlloc_traits::deallocate(a, current, 1);
        m_size -= 1;
        return iterator(static_cast<node_type*>(next));
    }

    iterator erase(const_iterator first, const_iterator last) {
        while (first != last) {
            first = this->erase(first);
        }
        return last;
    }

    void clear() noexcept {
        erase(begin(), end());
    }

    constexpr Alloc get_allocator() const { return m_allocator(); }

    constexpr T& front() { return *begin(); }
    constexpr const T& front() const { return *begin(); }
    constexpr T& back() { return *scratch::prev(end()); }
    constexpr const T& back() const { return *scratch::prev(end()); }

    constexpr iterator begin() noexcept { return m_after_tail().template m_plain_next<T>(); }
    constexpr const_iterator cbegin() noexcept { return m_after_tail().template m_plain_next<T>(); }
    constexpr const_iterator begin() const noexcept { return m_after_tail().template m_plain_next<T>(); }
    constexpr iterator end() noexcept { return &m_after_tail(); }
    constexpr const_iterator cend() noexcept { return &m_after_tail(); }
    constexpr const_iterator end() const noexcept { return &m_after_tail(); }

    constexpr size_t size() const noexcept { return m_size; }
    constexpr bool empty() const noexcept { return size() == 0; }

private:
    compressed_pair<detail::list_node_base<VoidPtr>, Alloc> m_head_and_allocator;
    size_t m_size = 0;

    void adopt_allocations_of(list& rhs) {
        this->m_after_tail().prev = std::exchange(rhs.m_after_tail().prev, {});
        this->m_after_tail().next = std::exchange(rhs.m_after_tail().next, {});
        this->m_size = std::exchange(rhs.m_size, 0);
        this->fix_up_internal_pointers();
        rhs.fix_up_internal_pointers();
    }

    void fix_up_internal_pointers() {
        if (m_size == 0) {
            m_after_tail().prev = pointer_to_m_after_tail();
            m_after_tail().next = pointer_to_m_after_tail();
        } else {
            m_after_tail().prev->next = pointer_to_m_after_tail();
            m_after_tail().next->prev = pointer_to_m_after_tail();
        }
    }

    auto& m_after_tail() noexcept { return get<0>(m_head_and_allocator); }
    const auto& m_after_tail() const noexcept { return get<0>(m_head_and_allocator); }
    Alloc& m_allocator() noexcept { return get<1>(m_head_and_allocator); }
    const Alloc& m_allocator() const noexcept { return get<1>(m_head_and_allocator); }

    auto pointer_to_m_after_tail() {
        return pointer_traits<node_base_pointer>::pointer_to(m_after_tail());
    }
};

template<class T, class A>
void swap(list<T, A>& lhs, list<T, A>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace scratch
