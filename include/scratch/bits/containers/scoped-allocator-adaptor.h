#pragma once

#include "scratch/bits/traits-classes/allocator-traits.h"
#include "scratch/bits/type-traits/allocator-arg.h"
#include "scratch/bits/type-traits/conditional.h"
#include "scratch/bits/type-traits/integral-constant.h"

#include <utility>

namespace scratch {

template<class OuterAlloc, class... InnerAllocs>
class scoped_allocator_adaptor {
    using OuterTraits = allocator_traits<OuterAlloc>;
public:
    using outer_allocator_type = OuterAlloc;
    using inner_allocator_type = scoped_allocator_adaptor<InnerAllocs...>;

    using value_type = typename OuterTraits::value_type;
    using pointer = typename OuterTraits::pointer;
    using void_pointer = typename OuterTraits::void_pointer;
    using difference_type = typename OuterTraits::difference_type;
    using size_type = typename OuterTraits::size_type;

    using propagate_on_container_copy_assignment = bool_constant<
        OuterTraits::propagate_on_container_copy_assignment::value ||
        (... || allocator_traits<InnerAllocs>::propagate_on_container_copy_assignment::value)
    >;
    using propagate_on_container_move_assignment = bool_constant<
        OuterTraits::propagate_on_container_move_assignment::value ||
        (... || allocator_traits<InnerAllocs>::propagate_on_container_move_assignment::value)
    >;
    using propagate_on_container_swap = bool_constant<
        OuterTraits::propagate_on_container_swap::value ||
        (... || allocator_traits<InnerAllocs>::propagate_on_container_swap::value)
    >;

    using is_always_equal = bool_constant<
        OuterTraits::is_always_equal::value &&
        (... && allocator_traits<InnerAllocs>::is_always_equal::value)
    >;

    // These flags should always be set consistently with each other.
    // If any of them don't match, there's probably a problem.
    static_assert(propagate_on_container_copy_assignment::value == propagate_on_container_swap::value);
    static_assert(propagate_on_container_move_assignment::value == propagate_on_container_swap::value);

    scoped_allocator_adaptor() = default;
    scoped_allocator_adaptor(scoped_allocator_adaptor&&) = default;
    scoped_allocator_adaptor(const scoped_allocator_adaptor&) = default;
    scoped_allocator_adaptor& operator=(scoped_allocator_adaptor&&) = default;
    scoped_allocator_adaptor& operator=(const scoped_allocator_adaptor&) = default;

    template<class... Us>
    explicit scoped_allocator_adaptor(const scoped_allocator_adaptor<Us...>& rhs) noexcept :
        m_outer(rhs.outer_allocator()), m_inner(rhs.inner_allocator()) {}

    template<class... Us>
    explicit scoped_allocator_adaptor(scoped_allocator_adaptor<Us...>&& rhs) noexcept :
        m_outer(std::move(rhs.outer_allocator())), m_inner(std::move(rhs.inner_allocator())) {}

    scoped_allocator_adaptor(OuterAlloc outerAlloc, InnerAllocs... innerAllocs) noexcept :
        m_outer(std::move(outerAlloc)), m_inner(std::move(innerAllocs)...) {}

    outer_allocator_type& outer_allocator() noexcept { return m_outer; }
    const outer_allocator_type& outer_allocator() const noexcept { return m_outer; }
    inner_allocator_type& inner_allocator() noexcept { return m_inner; }
    const inner_allocator_type& inner_allocator() const noexcept { return m_inner; }

    pointer allocate(size_type n) { return OuterTraits::allocate(m_outer, n); }
    void deallocate(pointer p, size_type n) { OuterTraits::deallocate(m_outer, p, n); }
    size_type max_size() const { return OuterTraits::max_size(m_outer); }
    void destroy(value_type *p) { return OuterTraits::destroy(m_outer, p); }

    template<class... Args>
    void construct(value_type *p, Args&&... args) {
        if constexpr (!uses_allocator_v<value_type, inner_allocator_type>) {
            OuterTraits::construct(m_outer, p, std::forward<Args>(args)...);
        } else if constexpr (is_constructible_v<value_type, allocator_arg_t, inner_allocator_type, Args&&...>) {
            OuterTraits::construct(m_outer, p, allocator_arg, m_inner, std::forward<Args>(args)...);
        } else if constexpr (is_constructible_v<value_type, Args&&..., inner_allocator_type&>) {
            OuterTraits::construct(m_outer, p, std::forward<Args>(args)..., m_inner);
        } else {
            static_assert(false_v<Args...>, "value_type is allocator-aware, but is not constructible with these argument types");
        }
    }

    scoped_allocator_adaptor select_on_container_copy_construction() const {
        return scoped_allocator_adaptor(
            OuterTraits::select_on_container_copy_construction(m_outer),
            allocator_traits<inner_allocator_type>::select_on_container_copy_construction(m_inner)
        );
    }

    template<class U> using rebind = scoped_allocator_adaptor<typename OuterTraits::template rebind_alloc<U>, InnerAllocs...>;

private:
    explicit scoped_allocator_adaptor(outer_allocator_type o, inner_allocator_type i) :
        m_outer(std::move(o)), m_inner(std::move(i)) {}

    outer_allocator_type m_outer;
    inner_allocator_type m_inner;
};

template<class OuterAlloc>
class scoped_allocator_adaptor<OuterAlloc> {
    using OuterTraits = allocator_traits<OuterAlloc>;
public:
    using outer_allocator_type = OuterAlloc;
    using inner_allocator_type = scoped_allocator_adaptor<OuterAlloc>;

    using value_type = typename OuterTraits::value_type;
    using pointer = typename OuterTraits::pointer;
    using void_pointer = typename OuterTraits::void_pointer;
    using difference_type = typename OuterTraits::difference_type;
    using size_type = typename OuterTraits::size_type;

    using propagate_on_container_copy_assignment = typename OuterTraits::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment = typename OuterTraits::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename OuterTraits::propagate_on_container_swap;

    using is_always_equal = typename OuterTraits::is_always_equal;

    // These flags should always be set consistently with each other.
    // If any of them don't match, there's probably a problem.
    static_assert(propagate_on_container_copy_assignment::value == propagate_on_container_swap::value);
    static_assert(propagate_on_container_move_assignment::value == propagate_on_container_swap::value);

    scoped_allocator_adaptor() = default;
    scoped_allocator_adaptor(scoped_allocator_adaptor&&) = default;
    scoped_allocator_adaptor(const scoped_allocator_adaptor&) = default;
    scoped_allocator_adaptor& operator=(scoped_allocator_adaptor&&) = default;
    scoped_allocator_adaptor& operator=(const scoped_allocator_adaptor&) = default;

    template<class U>
    explicit scoped_allocator_adaptor(const scoped_allocator_adaptor<U>& rhs) noexcept :
        m_outer(rhs.outer_allocator()) {}

    template<class U>
    explicit scoped_allocator_adaptor(scoped_allocator_adaptor<U>&& rhs) noexcept :
        m_outer(std::move(rhs.outer_allocator())) {}

    scoped_allocator_adaptor(OuterAlloc outerAlloc) noexcept :
        m_outer(std::move(outerAlloc)) {}

    outer_allocator_type& outer_allocator() noexcept { return m_outer; }
    const outer_allocator_type& outer_allocator() const noexcept { return m_outer; }
    inner_allocator_type& inner_allocator() noexcept { return *this; }
    const inner_allocator_type& inner_allocator() const noexcept { return *this; }

    pointer allocate(size_type n) { return OuterTraits::allocate(m_outer, n); }
    void deallocate(pointer p, size_type n) { OuterTraits::deallocate(m_outer, p, n); }
    size_type max_size() const { return OuterTraits::max_size(m_outer); }
    void destroy(value_type *p) { return OuterTraits::destroy(m_outer, p); }

    template<class... Args>
    void construct(value_type *p, Args&&... args) {
        if constexpr (!uses_allocator_v<value_type, inner_allocator_type>) {
            OuterTraits::construct(m_outer, p, std::forward<Args>(args)...);
        } else if constexpr (is_constructible_v<value_type, allocator_arg_t, inner_allocator_type, Args&&...>) {
            OuterTraits::construct(m_outer, p, allocator_arg, *this, std::forward<Args>(args)...);
        } else if constexpr (is_constructible_v<value_type, Args&&..., inner_allocator_type&>) {
            OuterTraits::construct(m_outer, p, std::forward<Args>(args)..., *this);
        } else {
            static_assert(false_v<Args...>, "value_type is allocator-aware, but is not constructible with these argument types");
        }
    }

    scoped_allocator_adaptor select_on_container_copy_construction() const {
        return scoped_allocator_adaptor(
            allocator_traits<OuterAlloc>::select_on_container_copy_construction(m_outer)
        );
    }

    template<class U> using rebind = scoped_allocator_adaptor<typename OuterTraits::template rebind_alloc<U>>;

private:
    outer_allocator_type m_outer;
};

template<class A>
bool operator==(const scoped_allocator_adaptor<A>& a, const scoped_allocator_adaptor<A>& b) noexcept {
    return a.outer_allocator() == b.outer_allocator();
}
template<class A, class... As>
bool operator==(const scoped_allocator_adaptor<A, As...>& a, const scoped_allocator_adaptor<A, As...>& b) noexcept {
    return a.outer_allocator() == b.outer_allocator() && a.inner_allocator() == b.inner_allocator();
}

template<class... As>
bool operator!=(const scoped_allocator_adaptor<As...>& a, const scoped_allocator_adaptor<As...>& b) noexcept {
    return !(a == b);
}

} // namespace scratch
