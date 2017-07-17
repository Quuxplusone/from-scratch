#pragma once

#include "scratch/bits/aligned-storage/max-align-t.h"

#include <cstddef>

namespace scratch::pmr {

template<class VoidPtr>
class fancy_memory_resource {
public:
    VoidPtr allocate(size_t bytes, size_t align = alignof(max_align_t)) {
        return do_allocate(bytes, align);
    }
    void deallocate(VoidPtr p, size_t bytes, size_t align = alignof(max_align_t)) {
        return do_deallocate(p, bytes, align);
    }
    bool is_equal(const fancy_memory_resource& rhs) const noexcept {
        return do_is_equal(rhs);
    }
    virtual ~fancy_memory_resource() = default;
private:
    virtual VoidPtr do_allocate(size_t bytes, size_t align) = 0;
    virtual void do_deallocate(VoidPtr p, size_t bytes, size_t align) = 0;
    virtual bool do_is_equal(const fancy_memory_resource& rhs) const noexcept = 0;
};

template<class P>
bool operator==(const fancy_memory_resource<P>& a, const fancy_memory_resource<P>& b) noexcept { return &a == &b || a.is_equal(b); }
template<class P>
bool operator!=(const fancy_memory_resource<P>& a, const fancy_memory_resource<P>& b) noexcept { return !(a == b); }

using memory_resource = fancy_memory_resource<void*>;

} // namespace scratch::pmr
