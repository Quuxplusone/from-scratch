#pragma once

#include "scratch/bits/aligned-storage/max-align-t.h"
#include "scratch/bits/containers/propagating-polymorphic-allocator.h"
#include "scratch/bits/containers/segmented-fancy-pointer.h"
#include "scratch/bits/memory-resource/memory-resource.h"
#include "scratch/bits/memory-resource/set-default-resource.h"
#include "scratch/bits/stdexcept/bad-alloc.h"
#include "scratch/bits/type-traits/integral-constant.h"

#include <cstddef>

namespace scratch::pmr::detail {

struct segmented_buffer_header {
    segmented_buffer_header *m_next;
    size_t m_size;
    size_t m_align;
    size_t m_allocated;
    size_t m_freed;
    unsigned char m_data[1];

    segmented_buffer_header *initialize(segmented_buffer_header *next, size_t bytes, size_t align) {
        m_next = next;
        m_size = bytes;
        m_align = align;
        m_allocated = offsetof(segmented_buffer_header, m_data);
        m_freed = offsetof(segmented_buffer_header, m_data);
        return this;
    }

    void *try_allocate(size_t bytes, size_t align) {
        if ((-m_allocated % align) > (m_size - m_allocated)) {
            return nullptr;
        }
        size_t new_offset = m_allocated + (-m_allocated % align);
        if (bytes > (m_size - new_offset)) {
            return nullptr;
        }
        m_allocated = new_offset + bytes;

        return (char*)this + (m_allocated - bytes);
    }

    bool deallocate(void *p, size_t bytes, size_t) {
        if ((char*)p + bytes == (char*)this + m_allocated) {
            m_allocated -= bytes;
        } else {
            m_freed += bytes;
            if (m_freed == m_allocated) {
                return true;  // maybe it's time to free this block
            }
        }
        return false;
    }
};

} // namespace scratch::pmr::detail

namespace scratch::pmr {

class segmented_resource : public fancy_memory_resource<segmented_fancy_pointer<void>> {
public:
    segmented_resource(const segmented_resource&) = delete;

    segmented_resource() : segmented_resource(1024, get_default_resource()) {}
    explicit segmented_resource(memory_resource *upstream) : segmented_resource(1024, upstream) {}
    explicit segmented_resource(size_t cap) : segmented_resource(cap, get_default_resource()) {}

    segmented_resource(size_t cap, memory_resource *upstream) {
        m_current = nullptr;
        m_next_buffer_size = 4 * sizeof(detail::segmented_buffer_header);
        if (m_next_buffer_size < cap) m_next_buffer_size = cap;
        m_upstream = upstream;
    }

    void release() {
        detail::segmented_buffer_header *h = m_current;
        while (h != nullptr) {
            auto next = h->m_next;
            m_upstream->deallocate(h, h->m_size, h->m_align);
            h = next;
        }
    }

    ~segmented_resource() {
        release();
    }

private:
    detail::segmented_buffer_header *m_current;
    size_t m_next_buffer_size;
    memory_resource *m_upstream;

    void allocate_another_buffer(size_t bytes, size_t align) {
        if (align < alignof(max_align_t)) align = alignof(max_align_t);
        if (bytes < m_next_buffer_size)   bytes = m_next_buffer_size;
        if (bytes < align + bytes) {
            bytes = align + bytes;
        } else {
            throw bad_alloc();
        }
        void *p = m_upstream->allocate(bytes, align);
        auto *h = reinterpret_cast<detail::segmented_buffer_header *>(p);
        m_current = h->initialize(m_current, bytes, align);
    }

private:
    segmented_fancy_pointer<void> do_allocate(size_t bytes, size_t align) override {
        for (auto *h = m_current; h != nullptr; h = h->m_next) {
            void *p = h->try_allocate(bytes, align);
            if (p != nullptr) {
                return segmented_fancy_pointer<void>(h, p);
            }
        }
        allocate_another_buffer(bytes, align);
        void *p = m_current->try_allocate(bytes, align);
        return segmented_fancy_pointer<void>(m_current, p);
    }

    void do_deallocate(segmented_fancy_pointer<void> p, size_t bytes, size_t align) override {
        auto *h = reinterpret_cast<detail::segmented_buffer_header *>(p.segment());
        h->deallocate(p.ptr(), bytes, align);
    }

    bool do_is_equal(const fancy_memory_resource<segmented_fancy_pointer<void>>& rhs) const noexcept override {
        return (this == &rhs);
    }
};

} // namespace scratch::pmr

namespace scratch {

template<class T>
using segmented_allocator = pmr::propagating_polymorphic_allocator<T, segmented_fancy_pointer<T>>;

} // namespace scratch
