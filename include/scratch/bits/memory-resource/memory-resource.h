#pragma once

#include "scratch/bits/aligned-storage/max-align-t.h"

#include <cstddef>

namespace scratch::pmr {

class memory_resource {
public:
    void *allocate(size_t bytes, size_t align = alignof(max_align_t)) {
        return do_allocate(bytes, align);
    }
    void deallocate(void *p, size_t bytes, size_t align = alignof(max_align_t)) {
        return do_deallocate(p, bytes, align);
    }
    bool is_equal(const memory_resource& rhs) const noexcept {
        return do_is_equal(rhs);
    }
    virtual ~memory_resource() = default;
private:
    virtual void *do_allocate(size_t bytes, size_t align) = 0;
    virtual void do_deallocate(void *p, size_t bytes, size_t align) = 0;
    virtual bool do_is_equal(const memory_resource& rhs) const noexcept = 0;
};

bool operator==(const memory_resource& a, const memory_resource& b) noexcept { return &a == &b || a.is_equal(b); }
bool operator!=(const memory_resource& a, const memory_resource& b) noexcept { return !(a == b); }

} // namespace scratch::pmr
