#pragma once

#include "scratch/bits/traits-classes/pointer-traits.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-static-castable.h"

#include <cstddef>

namespace scratch::pmr {

template<class T, class MemoryResource, class InternalVoidPtr = void*>
class fancy_polymorphic_allocator
{
    using MrPtr = decltype(declval<MemoryResource&>().allocate(0,0));
public:
    using value_type = T;
    using pointer = typename pointer_traits<MrPtr>::template rebind<T>;
    using void_pointer = MrPtr;
    using propagate_on_container_copy_assignment = false_type;
    using propagate_on_container_move_assignment = false_type;
    using propagate_on_container_swap = false_type;

    using resource_pointer = typename pointer_traits<InternalVoidPtr>::template rebind<MemoryResource>;

    // Polymorphic allocators do not propagate on container swap, or move, or copy.
    // This means that if you move-assign a pmr::vector, you'll see all the
    // elements getting moved from the rhs's arena into the lhs's arena (whatever
    // arena that might be); and swap will involve two such movements.

    fancy_polymorphic_allocator(MemoryResource *mr) : m_mr(mr) {}

    fancy_polymorphic_allocator(const fancy_polymorphic_allocator&) noexcept = default;
    fancy_polymorphic_allocator& operator=(const fancy_polymorphic_allocator&) = delete;

    template<class U>
    explicit fancy_polymorphic_allocator(const fancy_polymorphic_allocator<U, MemoryResource, InternalVoidPtr>& rhs) noexcept {
        m_mr = rhs.resource();
    }

    pointer allocate(size_t n) {
        return static_cast<pointer>(m_mr->allocate(n * sizeof(T), alignof(T)));
    }
    void deallocate(pointer p, size_t n) {
        m_mr->deallocate(static_cast<void_pointer>(p), n * sizeof(T), alignof(T));
    }
    resource_pointer resource() const { return m_mr; }

    fancy_polymorphic_allocator select_on_container_copy_construction() const {
        return fancy_polymorphic_allocator();
    }
private:
    resource_pointer m_mr;
};

template<class A, class B, class M, class V>
bool operator==(const fancy_polymorphic_allocator<A,M,V>& a, const fancy_polymorphic_allocator<B,M,V>& b) noexcept {
    return *a.resource() == *b.resource();
}
template<class A, class B, class M, class V>
bool operator!=(const fancy_polymorphic_allocator<A,M,V>& a, const fancy_polymorphic_allocator<B,M,V>& b) noexcept {
    return *a.resource() != *b.resource();
}

} // namespace scratch::pmr
