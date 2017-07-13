#pragma once

#include "scratch/bits/aligned-storage/max-align-t.h"
#include "scratch/bits/memory-resource/memory-resource.h"
#include "scratch/bits/memory-resource/set-default-resource.h"
#include "scratch/bits/stdexcept/bad-alloc.h"

#include <cstddef>

namespace scratch::pmr::detail {

struct monotonic_buffer_header {
    void *m_next;
    size_t m_size;
    size_t m_align;
    unsigned char m_data[1];

    void *initialize(void *next, size_t bytes, size_t align) {
        m_next = next;
        m_size = bytes;
        m_align = align;
        return &m_data;
    }
};

} // namespace scratch::pmr::detail

namespace scratch::pmr {

class monotonic_buffer_resource : public memory_resource {
public:
    monotonic_buffer_resource(const monotonic_buffer_resource&) = delete;

    monotonic_buffer_resource() : monotonic_buffer_resource(nullptr, 1024, get_default_resource()) {}
    explicit monotonic_buffer_resource(memory_resource *upstream) : monotonic_buffer_resource(nullptr, 1024, upstream) {}
    explicit monotonic_buffer_resource(size_t cap) : monotonic_buffer_resource(nullptr, cap, get_default_resource()) {}
    monotonic_buffer_resource(size_t cap, memory_resource *upstream) : monotonic_buffer_resource(nullptr, cap, upstream) {}
    monotonic_buffer_resource(void *p, size_t cap) : monotonic_buffer_resource(p, cap, get_default_resource()) {}

    monotonic_buffer_resource(void *p, size_t cap, memory_resource *upstream) {
        m_current_buffer = p;
        m_current_capacity = (p ? cap : 0);
        m_current_size = 0;
        m_next_buffer_size = 4 * sizeof(detail::monotonic_buffer_header);
        if (m_next_buffer_size < cap) m_next_buffer_size = cap;
        m_upstream = upstream;
        m_buffers_to_free = 0;
    }

    void release() {
        void *p = m_current_buffer;
        for (int i = 0; i < m_buffers_to_free; ++i) {
            p = (char*)p - offsetof(detail::monotonic_buffer_header, m_data);
            auto *h = reinterpret_cast<detail::monotonic_buffer_header *>(p);
            p = h->m_next;
            m_upstream->deallocate(h, h->m_size, h->m_align);
        }
    }

    ~monotonic_buffer_resource() {
        release();
    }

private:
    void *m_current_buffer;
    size_t m_current_capacity;
    size_t m_current_size;
    size_t m_next_buffer_size;
    memory_resource *m_upstream;
    int m_buffers_to_free;

    void allocate_another_buffer(size_t bytes, size_t align) {
        if (align < alignof(max_align_t)) align = alignof(max_align_t);
        if (bytes < m_next_buffer_size)   bytes = m_next_buffer_size;
        if (bytes < align + bytes) {
            bytes = align + bytes;
        } else {
            throw bad_alloc();
        }
        void *p = m_upstream->allocate(bytes, align);

        auto *h = reinterpret_cast<detail::monotonic_buffer_header *>(p);
        m_current_buffer = h->initialize(m_current_buffer, bytes, align);
        m_current_capacity = bytes - ((char*)m_current_buffer - (char*)p);
        m_current_size = 0;
        m_buffers_to_free += 1;
    }

    void *do_allocate(size_t bytes, size_t align) override {
        if ((-m_current_size % align) > (m_current_capacity - m_current_size)) {
            allocate_another_buffer(bytes, align);
        }
        m_current_size += (-m_current_size % align);
        if (bytes > (m_current_capacity - m_current_size)) {
            allocate_another_buffer(bytes, align);
            m_current_size += (-m_current_size % align);
        }
        m_current_size += bytes;

        return (char*)m_current_buffer + (m_current_size - bytes);
    }

    void do_deallocate(void *, size_t, size_t) override {}

    bool do_is_equal(const memory_resource& rhs) const noexcept override {
        return (this == &rhs);
    }
};

} // namespace scratch::pmr
