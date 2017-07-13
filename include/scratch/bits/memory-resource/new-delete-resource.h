#pragma once

#include "scratch/bits/memory-resource/memory-resource.h"

#include <cstddef>
#include <new>

namespace scratch::pmr::detail {

class new_delete_memory_resource : public memory_resource {
    void *do_allocate(size_t bytes, size_t align) override {
        return ::operator new(bytes, std::align_val_t(align));
    }
    void do_deallocate(void *p, size_t bytes, size_t align) override {
        ::operator delete(p, bytes, std::align_val_t(align));
    }
    bool do_is_equal(const memory_resource& rhs) const noexcept override {
        return (this == &rhs);
    }
};

} // namespace scratch::pmr::detail

namespace scratch::pmr {

inline memory_resource *new_delete_resource() noexcept {
    static detail::new_delete_memory_resource instance;
    return &instance;
}

} // namespace scratch::pmr
