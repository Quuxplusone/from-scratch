#pragma once

#include "scratch/bits/memory-resource/memory-resource.h"
#include "scratch/bits/stdexcept/bad-alloc.h"

#include <cstddef>

namespace scratch::pmr::detail {

class null_memory_resource : public memory_resource {
    void *do_allocate(size_t bytes, size_t align) override {
        throw bad_alloc();
    }
    void do_deallocate(void *p, size_t bytes, size_t align) override {}
    bool do_is_equal(const memory_resource& rhs) const noexcept override {
        return (this == &rhs);
    }
};

} // namespace scratch::pmr::detail

namespace scratch::pmr {

inline memory_resource *null_memory_resource() noexcept {
    static detail::null_memory_resource instance;
    return &instance;
}

} // namespace scratch::pmr
