#pragma once

#include "scratch/bits/aligned-storage/max-align-t.h"
#include "scratch/bits/memory-resource/memory-resource.h"
#include "scratch/bits/stdexcept/bad-alloc.h"

#include <cstddef>

namespace scratch::pmr {

template<size_t Align>
class aligned_counting_buffer_resource : public memory_resource {
public:
    aligned_counting_buffer_resource(const aligned_counting_buffer_resource&) = delete;

    aligned_counting_buffer_resource(void *p, size_t cap) {
        m_buffer = static_cast<char *>(p);
        m_capacity = cap;
    }

    size_t allocated_bytes() const {
        return (m_size - m_freed);
    }

    void release() {
        m_size = 0;
        m_freed = 0;
    }

private:
    char *m_buffer;
    size_t m_capacity;
    size_t m_size = 0;
    size_t m_freed = 0;

    void *do_allocate(size_t bytes, size_t align) override {
        if (align > Align ||
            bytes + (-bytes % Align) < bytes ||
            bytes + (-bytes % Align) > (m_capacity - m_size))
        {
            throw bad_alloc();
        }
        bytes += (-bytes % Align);
        m_size += bytes;
        return m_buffer + (m_size - bytes);
    }

    void do_deallocate(void *p, size_t bytes, size_t) override {
        bytes += (-bytes % Align);
        if (p == m_buffer + (m_size - bytes)) {
            m_size -= bytes;
        } else {
            m_freed += bytes;
            if (allocated_bytes() == 0) {
                release();
            }
        }
    }

    bool do_is_equal(const memory_resource& rhs) const noexcept override {
        return (this == &rhs);
    }
};

using counting_buffer_resource = aligned_counting_buffer_resource<alignof(max_align_t)>;

} // namespace scratch::pmr
