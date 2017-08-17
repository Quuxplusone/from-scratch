#pragma once

#include "scratch/bits/containers/offset-ptr.h"
#include "scratch/bits/traits-classes/allocator-traits.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-fooible.h"
#include "scratch/bits/type-traits/is-static-castable.h"

#include <utility>

namespace scratch {

template<class InnerAlloc>
class offset_allocator_adaptor {
    using InnerTraits = allocator_traits<InnerAlloc>;
    using InnerPtr = typename InnerTraits::pointer;
public:
    using inner_allocator_type = InnerAlloc;

    using value_type = typename InnerTraits::value_type;
    using pointer = offset_ptr<value_type>;
    using void_pointer = offset_ptr<void>;
    using difference_type = typename InnerTraits::difference_type;
    using size_type = typename InnerTraits::size_type;

    static_assert(is_static_castable_v<InnerPtr, value_type*>);
    static_assert(is_static_castable_v<value_type*, InnerPtr>);

    using propagate_on_container_copy_assignment = typename InnerTraits::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment = typename InnerTraits::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename InnerTraits::propagate_on_container_swap;
    using is_always_equal = typename InnerTraits::is_always_equal;

    offset_allocator_adaptor() = default;
    offset_allocator_adaptor(offset_allocator_adaptor&&) = default;
    offset_allocator_adaptor(const offset_allocator_adaptor&) = default;
    offset_allocator_adaptor& operator=(offset_allocator_adaptor&&) = default;
    offset_allocator_adaptor& operator=(const offset_allocator_adaptor&) = default;

    template<class A>
    explicit offset_allocator_adaptor(const offset_allocator_adaptor<A>& rhs) noexcept :
        m_inner(rhs.inner_allocator()) {}

    template<class A>
    explicit offset_allocator_adaptor(offset_allocator_adaptor<A>&& rhs) noexcept :
        m_inner(std::move(rhs.inner_allocator())) {}

    template<class... Args, class = enable_if_t<is_constructible_v<InnerAlloc, Args&&...>>>
    offset_allocator_adaptor(Args&&... args) noexcept :
        m_inner(std::forward<Args>(args)...) {}

    inner_allocator_type& inner_allocator() noexcept { return m_inner; }
    const inner_allocator_type& inner_allocator() const noexcept { return m_inner; }

    pointer allocate(size_type n) {
        InnerPtr ip = InnerTraits::allocate(m_inner, n);
        return static_cast<pointer>(static_cast<value_type*>(ip));
    }
    void deallocate(pointer p, size_type n) {
        InnerPtr ip = static_cast<InnerPtr>(static_cast<value_type*>(p));
        InnerTraits::deallocate(m_inner, ip, n);
    }
    size_type max_size() const { return InnerTraits::max_size(m_inner); }
    void destroy(value_type *p) { return InnerTraits::destroy(m_inner, p); }

    offset_allocator_adaptor select_on_container_copy_construction() const {
        return offset_allocator_adaptor(
            InnerTraits::select_on_container_copy_construction(m_inner)
        );
    }

    template<class U> using rebind = offset_allocator_adaptor<typename InnerTraits::template rebind_alloc<U>>;

private:
    inner_allocator_type m_inner;
};

template<class A, class B>
bool operator==(const offset_allocator_adaptor<A>& a, const offset_allocator_adaptor<B>& b) noexcept {
    return a.inner_allocator() == b.inner_allocator();
}

template<class A, class B>
bool operator!=(const offset_allocator_adaptor<A>& a, const offset_allocator_adaptor<B>& b) noexcept {
    return a.inner_allocator() != b.inner_allocator();
}

} // namespace scratch
