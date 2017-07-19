#pragma once

#include "scratch/bits/memory-resource/memory-resource.h"
#include "scratch/bits/memory-resource/set-default-resource.h"
#include "scratch/bits/smart-ptrs/shared-ptr.h"
#include "scratch/bits/traits-classes/pointer-traits.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"

#include <cstddef>
#include <utility>

namespace scratch::pmr {

template<class T, class VoidPtr = void*>
class shared_polymorphic_allocator {
public:
    using value_type = T;
    using pointer = typename pointer_traits<VoidPtr>::template rebind<T>;
    using void_pointer = VoidPtr;
    using propagate_on_container_copy_assignment = true_type;
    using propagate_on_container_move_assignment = true_type;
    using propagate_on_container_swap = true_type;

    template<bool B = is_same_v<VoidPtr, void*>, class = enable_if_t<B>>
    shared_polymorphic_allocator() : m_mr(get_default_resource()) {}

    shared_polymorphic_allocator(shared_ptr<fancy_memory_resource<void_pointer>> mr, bool reseatable) :
        m_mr(std::move(mr)), m_reseatable(reseatable) {}

    shared_polymorphic_allocator(const shared_polymorphic_allocator&) = default;
    shared_polymorphic_allocator(shared_polymorphic_allocator&&) = default;

    shared_polymorphic_allocator& operator=(const shared_polymorphic_allocator& rhs) {
        if (m_reseatable) {
            m_mr = rhs.m_mr;
        }
    }

    shared_polymorphic_allocator& operator=(shared_polymorphic_allocator&& rhs) {
        if (m_reseatable) {
            m_mr = std::move(rhs.m_mr);
        }
    }

    template<class U>
    explicit shared_polymorphic_allocator(const shared_polymorphic_allocator<U, VoidPtr>& rhs) noexcept {
        m_mr = rhs.m_mr;
    }

    pointer allocate(size_t n) { return static_cast<pointer>(m_mr->allocate(n * sizeof(T), alignof(T))); }
    void deallocate(pointer p, size_t n) { m_mr->deallocate(static_cast<void_pointer>(p), n * sizeof(T), alignof(T)); }
    fancy_memory_resource<void_pointer> *resource() const { return m_mr.get(); }
    bool is_reseatable() const { return m_reseatable; }

private:
    shared_ptr<fancy_memory_resource<void_pointer>> m_mr;
    bool m_reseatable;

    template<class U>
    friend class shared_polymorphic_allocator<U, VoidPtr>;
};

template<class A, class B, class P>
bool operator==(const shared_polymorphic_allocator<A,P>& a, const shared_polymorphic_allocator<B,P>& b) noexcept {
    return *a.resource() == *b.resource();
}
template<class A, class B, class P>
bool operator!=(const shared_polymorphic_allocator<A,P>& a, const shared_polymorphic_allocator<B,P>& b) noexcept {
    return *a.resource() != *b.resource();
}

} // namespace scratch::pmr
