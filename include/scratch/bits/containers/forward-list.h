#pragma once

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
    template<typename T, typename A = allocator<T>> class forward_list;  // forward declaration
} // namespace scratch

namespace scratch::detail {

template<typename VoidPtr> struct forward_list_node_base;  // forward declaration
template<typename VoidPtr, typename T> struct forward_list_node;  // forward declaration

template<class VoidPtr>
using forward_list_node_base_ptr = typename pointer_traits<VoidPtr>::template rebind<forward_list_node_base<VoidPtr>>;

template<class VoidPtr, class T>
using forward_list_node_ptr = typename pointer_traits<VoidPtr>::template rebind<forward_list_node<VoidPtr, T>>;

template<class VoidPtr>
struct forward_list_node_base {
    forward_list_node_base_ptr<VoidPtr> next;

    explicit forward_list_node_base(forward_list_node_base_ptr<VoidPtr> n) : next(n) {}

    template<typename T>
    auto m_fancy_next() const { return static_cast<forward_list_node_ptr<VoidPtr, T>>(this->next); }

    template<typename T>
    auto m_plain_next() const { return static_cast<forward_list_node<VoidPtr, T>*>(this->m_fancy_next<T>()); }
};

template<class VoidPtr, class T>
struct forward_list_node : compressed_element<T, 0>, forward_list_node_base<VoidPtr>
{
    template<typename... Args>
    explicit forward_list_node(forward_list_node_base_ptr<VoidPtr> n, Args&&... args) :
        compressed_element<T, 0>(std::forward<Args>(args)...),
        forward_list_node_base<VoidPtr>(n)
    {}

    T& m_data() { return this->get_element(index_constant<0>{}); }
    const T& m_data() const { return this->get_element(index_constant<0>{}); }
};

template<typename VoidPtr, typename T, bool IsConst>
class forward_list_iterator
{
    using node_type = forward_list_node<VoidPtr, T>;
public:
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = conditional_t<IsConst, const value_type, value_type>*;
    using reference = conditional_t<IsConst, const value_type, value_type>&;
    using iterator_category = forward_iterator_tag;

    forward_list_iterator() = default;

    auto& operator++() { m_ptr = m_ptr->template m_plain_next<T>(); return *this; }
    auto operator++(int) { auto old = *this; ++*this; return old; }

    reference operator*() const { return static_cast<node_type *>(m_ptr)->m_data(); }
    pointer operator->() const { return &static_cast<node_type *>(m_ptr)->m_data(); }

    operator forward_list_iterator<VoidPtr, T, true>() const noexcept { return forward_list_iterator<VoidPtr, T, true>(m_ptr); }

    template<bool C> bool operator==(const forward_list_iterator<VoidPtr,T,C>& rhs) const noexcept { return m_ptr == rhs.m_ptr; }
    template<bool C> bool operator!=(const forward_list_iterator<VoidPtr,T,C>& rhs) const noexcept { return m_ptr != rhs.m_ptr; }
private:
    template<class U, class Alloc> friend class scratch::forward_list;
    friend class forward_list_iterator<VoidPtr, T, !IsConst>;

    forward_list_iterator(const forward_list_node_base<VoidPtr> *p) : m_ptr(const_cast<forward_list_node_base<VoidPtr> *>(p)) {}
    node_type *get_node_pointer() { return static_cast<node_type *>(m_ptr); }
    operator forward_list_iterator<VoidPtr, T, false>() const noexcept { return forward_list_iterator<VoidPtr, T, false>(m_ptr); }

    forward_list_node_base<VoidPtr> *m_ptr = nullptr;
};

} // namespace scratch::detail

namespace scratch {

template<class T, class Alloc>
class forward_list {
    using Alloc_traits = allocator_traits<Alloc>;
    using VoidPtr = typename Alloc_traits::void_pointer;
    using node_base_pointer = detail::forward_list_node_base_ptr<VoidPtr>;
    using node_type = detail::forward_list_node<VoidPtr, T>;
    using node_pointer = detail::forward_list_node_ptr<VoidPtr, T>;
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
    using iterator = detail::forward_list_iterator<VoidPtr, T, false>;
    using const_iterator = detail::forward_list_iterator<VoidPtr, T, true>;

    forward_list() noexcept : forward_list(Alloc{}) {}
    explicit forward_list(size_t count) : forward_list(count, Alloc{}) {}
    explicit forward_list(size_t count, const T& value) : forward_list(count, value, Alloc{}) {}

    template<class It, class = enable_if_t<is_input_iterator_v<It>>>
    explicit forward_list(It first, It last) : forward_list(first, last, Alloc{}) {}

    explicit forward_list(Alloc a) noexcept :
        m_head_and_allocator(
            detail::forward_list_node_base<VoidPtr>{node_pointer{}},
            std::move(a)
        )
    {}

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

    forward_list(const forward_list& rhs) : forward_list(rhs, Alloc_traits::select_on_container_copy_construction(rhs.m_allocator())) {}

    forward_list(const forward_list& rhs, Alloc a) : forward_list(std::move(a)) {
        this->assign(rhs.begin(), rhs.end());
    }

    forward_list(forward_list&& rhs) noexcept : forward_list(std::move(rhs.m_allocator())) {
        // we can adopt the new allocator and its memory wholesale
        this->adopt_allocations_of(rhs);
    }

    forward_list(forward_list&& rhs, Alloc a) noexcept : forward_list(std::move(a)) {
        if (this->m_allocator() == rhs.m_allocator()) {
            // we can adopt the new allocator and its memory wholesale
            this->adopt_allocations_of(rhs);
        } else {
            // we were given a different allocator and thus cannot adopt this memory
            this->assign(rhs.begin(), rhs.end());
            rhs.clear();
        }
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
            auto temp = std::move(*this);  // save the pointer to my data
            *this = std::move(rhs);  // copy rhs's data into my heap, and clear rhs
            rhs = std::move(temp);  // copy my data into rhs's heap
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
        node_type *prev = it.get_node_pointer();
        node_pointer next = prev->template m_fancy_next<T>();

        NodeAlloc a(m_allocator());
        node_pointer p = NodeAlloc_traits::allocate(a, 1);
        try {
            NodeAlloc_traits::construct(a, static_cast<node_type*>(p), next, std::forward<Args>(args)...);
            m_size += 1;
        } catch (...) {
            NodeAlloc_traits::deallocate(a, p, 1);
            throw;
        }
        prev->next = p;
        return iterator(prev->template m_plain_next<T>());
    }

    iterator erase_after(const_iterator it) {
        node_type *prev = it.get_node_pointer();
        node_pointer next = prev->template m_fancy_next<T>();

        NodeAlloc a(m_allocator());
        prev->next = next->next;
        NodeAlloc_traits::destroy(a, static_cast<node_type*>(next));
        NodeAlloc_traits::deallocate(a, next, 1);
        m_size -= 1;
        return iterator(prev->template m_plain_next<T>());
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

    constexpr iterator begin() noexcept { return m_before_head().template m_plain_next<T>(); }
    constexpr const_iterator cbegin() noexcept { return m_before_head().template m_plain_next<T>(); }
    constexpr const_iterator begin() const noexcept { return m_before_head().template m_plain_next<T>(); }
    constexpr iterator end() noexcept { return nullptr; }
    constexpr const_iterator cend() noexcept { return nullptr; }
    constexpr const_iterator end() const noexcept { return nullptr; }

    constexpr size_t size() const noexcept { return m_size; }
    constexpr bool empty() const noexcept { return size() == 0; }

private:
    compressed_pair<detail::forward_list_node_base<VoidPtr>, Alloc> m_head_and_allocator;
    size_t m_size = 0;

    void adopt_allocations_of(forward_list& rhs) {
        this->m_before_head().next = std::exchange(rhs.m_before_head().next, {});
        this->m_size = std::exchange(rhs.m_size, 0);
    }

    auto& m_before_head() noexcept { return get<0>(m_head_and_allocator); }
    const auto& m_before_head() const noexcept { return get<0>(m_head_and_allocator); }
    Alloc& m_allocator() noexcept { return get<1>(m_head_and_allocator); }
    const Alloc& m_allocator() const noexcept { return get<1>(m_head_and_allocator); }
};

template<class T, class A>
void swap(forward_list<T, A>& lhs, forward_list<T, A>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace scratch
