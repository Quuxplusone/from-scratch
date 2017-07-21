#pragma once

#include "scratch/bits/memory-resource/memory-resource.h"
#include "scratch/bits/stdexcept/bad-alloc.h"

#include <cstddef>

namespace scratch::pmr {

class simple_buffer_resource : public memory_resource {
public:
    simple_buffer_resource(const simple_buffer_resource&) = delete;

    simple_buffer_resource(void *p, size_t cap) {
        m_buffer = static_cast<char *>(p);
        m_capacity = cap;
    }

    void release() {
        m_size = 0;
    }

private:
    char *m_buffer;
    size_t m_capacity;
    size_t m_size = 0;

    void *do_allocate(size_t bytes, size_t align) override {
        if ((-m_size % align) > (m_capacity - m_size) ||
            bytes > (m_capacity - m_size - (-m_size % align)))
        {
            throw bad_alloc();
        }
        m_size += (-m_size % align) + bytes;
        return m_buffer + (m_size - bytes);
    }

    void do_deallocate(void *, size_t, size_t) override {}

    bool do_is_equal(const memory_resource& rhs) const noexcept override {
        return (this == &rhs);
    }
};

} // namespace scratch::pmr
