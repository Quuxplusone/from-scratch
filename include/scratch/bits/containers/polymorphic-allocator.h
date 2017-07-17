#pragma once

#include "scratch/bits/memory-resource/memory-resource.h"
#include "scratch/bits/memory-resource/set-default-resource.h"
#include "scratch/bits/traits-classes/pointer-traits.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"

#include <cstddef>

namespace scratch::pmr {

template<class T, class VoidPtr = void*>
class polymorphic_allocator {
public:
    using value_type = T;
    using pointer = typename pointer_traits<VoidPtr>::template rebind<T>;
    using void_pointer = VoidPtr;
    using propagate_on_container_copy_assignment = false_type;
    using propagate_on_container_move_assignment = false_type;
    using propagate_on_container_swap = false_type;

    // Polymorphic allocators do not propagate on container swap, or move, or copy.
    // This means that if you move-assign a pmr::vector, you'll see all the
    // elements getting moved from the rhs's arena into the lhs's arena (whatever
    // arena that might be); and swap will involve two such movements.

    template<bool B = is_same_v<VoidPtr, void*>, class = enable_if_t<B>>
    polymorphic_allocator() : m_mr(get_default_resource()) {}

    polymorphic_allocator(fancy_memory_resource<void_pointer> *mr) : m_mr(mr) {}

    polymorphic_allocator(const polymorphic_allocator&) noexcept = default;
    polymorphic_allocator& operator=(const polymorphic_allocator&) = delete;

    template<class U>
    explicit polymorphic_allocator(const polymorphic_allocator<U, VoidPtr>& rhs) noexcept {
        m_mr = rhs.resource();
    }

    pointer allocate(size_t n) { return static_cast<pointer>(m_mr->allocate(n * sizeof(T), alignof(T))); }
    void deallocate(pointer p, size_t n) { m_mr->deallocate(static_cast<void_pointer>(p), n * sizeof(T), alignof(T)); }
    fancy_memory_resource<void_pointer> *resource() const { return m_mr; }

    polymorphic_allocator select_on_container_copy_construction() const {
        return polymorphic_allocator();
    }
private:
    fancy_memory_resource<void_pointer> *m_mr;
};

template<class A, class B, class P>
bool operator==(const polymorphic_allocator<A,P>& a, const polymorphic_allocator<B,P>& b) noexcept {
    return *a.resource() == *b.resource();
}
template<class A, class B, class P>
bool operator!=(const polymorphic_allocator<A,P>& a, const polymorphic_allocator<B,P>& b) noexcept {
    return *a.resource() != *b.resource();
}

} // namespace scratch::pmr
