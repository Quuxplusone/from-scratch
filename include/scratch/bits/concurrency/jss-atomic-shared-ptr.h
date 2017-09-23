
#include "scratch/bits/smart-ptrs/default-delete.h"
#include "scratch/bits/smart-ptrs/unique-ptr.h"
#include "scratch/bits/type-traits/compiler-magic.h"

#include <atomic>
#include <utility>

// Implementation of atomic_shared_ptr as per N4162
// (http://isocpp.org/files/papers/N4162.pdf)
//
// Copyright (c) 2014, Just Software Solutions Ltd
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or
// without modification, are permitted provided that the
// following conditions are met:
//
// 1. Redistributions of source code must retain the above
// copyright notice, this list of conditions and the following
// disclaimer.
//
// 2. Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following
// disclaimer in the documentation and/or other materials
// provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of
// its contributors may be used to endorse or promote products
// derived from this software without specific prior written
// permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

namespace scratch::jss {

struct counter;
struct counted_ptr;
class ptr_extension_block;
class shared_ptr_header_block_base;
template<class P, class D> class shared_ptr_header_separate;
template<class T> class shared_ptr;
template<class T> class weak_ptr;
struct local_access;
template<class T> class atomic_shared_ptr;


template<class T>
static constexpr bool is_lockfree_atomic_v =
    is_trivially_copyable_v<T> &&
    sizeof(T) <= 16 &&
    std::atomic<T>::is_always_lock_free;

// A "counter" represents a reference count as stored in the shared_ptr_control_block.
// "count" represents the reference count, and "external_counters" is (something like)
// the number of atomic_shared_ptrs that reference this block.
struct counter {
    int external_counters = 0;
    int count = 1;
};
static_assert(is_lockfree_atomic_v<counter>);


class ptr_extension_block {
public:
    using uindex_type = unsigned int;
    static constexpr uindex_type CAST_POINTER_COUNT = 3;
    std::atomic<void*> cast_pointers[CAST_POINTER_COUNT] = {};
    std::atomic<ptr_extension_block*> cp_extension {nullptr};

    ptr_extension_block() = default;

    uindex_type get_index_from_ptr(void *p)
    {
        for (uindex_type i=0; i < CAST_POINTER_COUNT; ++i){
            void *entry = cast_pointers[i].load();
            if (entry == nullptr) {
                // This entry is blank! Try to set it to "p".
                // If we fail, at least we won't get "nullptr" a second time.
                if (cast_pointers[i].compare_exchange_strong(entry, p)) {
                    entry = p;
                }
            }
            if (entry == p) {
                return i;
            }
        }
        ptr_extension_block *extension = cp_extension.load();
        if (extension == nullptr) {
            ptr_extension_block *new_extension = new ptr_extension_block;
            if (cp_extension.compare_exchange_strong(extension, new_extension)) {
                extension = new_extension;
            } else {
                delete new_extension;
            }
        }
        return extension->get_index_from_ptr(p) + CAST_POINTER_COUNT;
    }

    void *get_ptr_from_index(uindex_type index)
    {
        if (index < CAST_POINTER_COUNT) {
            return cast_pointers[index].load();
        } else {
            return cp_extension.load()->get_ptr_from_index(index - CAST_POINTER_COUNT);
        }
    }

    ~ptr_extension_block()
    {
        delete cp_extension.load();
    }
};

class shared_ptr_header_block_base {
    std::atomic<counter> count{counter{}};
    std::atomic<int> weak_count{1};
    ptr_extension_block cp_extension{};

public:
    int use_count() {
        counter c = count.load();
        return c.count + (c.external_counters ? 1 : 0);
    }

    auto get_index_from_ptr(void *p) { return cp_extension.get_index_from_ptr(p); }
    template<typename T> auto get_ptr_from_index(ptr_extension_block::uindex_type index) {
        return static_cast<T*>(cp_extension.get_ptr_from_index(index));
    }

    shared_ptr_header_block_base() = default;
    virtual void do_delete() = 0;
    virtual ~shared_ptr_header_block_base() = default;

    void inc_weak_count() {
        ++weak_count;
    }
    void dec_weak_count() {
        int new_weak_count = --weak_count;
        if (new_weak_count == 0) delete this;
    }

    void inc_count() {
        // Increment "count.count"; don't change "count.external_counters"
        counter expected = count.load();
        counter desired;
        do {
            desired = expected;
            desired.count += 1;
        } while (!count.compare_exchange_weak(expected, desired));
    }

    bool try_inc_count() {
        // Increment "count.count" only if use_count() is non-zero
        counter expected = count.load();
        counter desired;
        do {
            if (expected.count == 0 && expected.external_counters == 0) return false;
            desired = expected;
            desired.count += 1;
        } while (!count.compare_exchange_weak(expected, desired));
        return true;
    }

    void dec_count() {
        // Decrement "count.count"; don't change "count.external_counters"
        counter expected = count.load();
        counter desired;
        do {
            desired = expected;
            desired.count -= 1;
        } while (!count.compare_exchange_weak(expected, desired));
        if (desired.count == 0 && desired.external_counters == 0) {
            do_delete();
            dec_weak_count();
        }
    }

    void inc_external_counters(int external_count) {
        counter expected = count.load();
        counter desired;
        do {
            desired = expected;
            desired.external_counters += external_count;
        } while (!count.compare_exchange_weak(expected, desired));
    }

    void dec_external_counters() {
        counter expected = count.load();
        counter desired;
        do {
            desired = expected;
            desired.external_counters -= 1;
        } while (!count.compare_exchange_weak(expected, desired));
        if (desired.count == 0 && desired.external_counters == 0) {
            do_delete();
            dec_weak_count();
        }
    }
};


// A "counted_ptr" represents the value of a shared_ptr (m_ptr and m_header).
// "m_header/m_index" represents the shared_ptr's value.
// "m_local_access_count" is the count of local_access objects still looking at this m_header.
// The only reason for "m_index" is that we need to squeeze the "m_ptr" pointer into <64 bits
// so that we have room for "m_local_access_count".
struct counted_ptr {
    unsigned m_local_access_count = 0;
    ptr_extension_block::uindex_type m_index = 0;
    shared_ptr_header_block_base *m_header = nullptr;

    explicit counted_ptr() = default;
    explicit counted_ptr(shared_ptr_header_block_base *header, int index) {
        m_index = index;
        m_header = header;
    }

    void destroy() {
        // assert(m_local_access_count >= 0);
        if (m_header) {
            m_header->inc_external_counters(m_local_access_count);
            m_header->dec_count();
        }
    }
};
static_assert(is_lockfree_atomic_v<counted_ptr>);  // If this fails, add "-mcx16" to your command line.


template<class P, class D>
class shared_ptr_header_separate : public shared_ptr_header_block_base
{
    P m_ptr;
    D m_deleter;
public:
    explicit shared_ptr_header_separate(P p) : m_ptr(p) {}
    template<class E> explicit shared_ptr_header_separate(P p, E& d) : m_deleter(d), m_ptr(p) {}

    void do_delete() override { m_deleter(m_ptr); }
};

template<class T>
class shared_ptr {
    T *m_ptr = nullptr;
    shared_ptr_header_block_base *m_header = nullptr;

    template<typename U> friend class atomic_shared_ptr;
    template<typename U> friend class shared_ptr;
    template<typename U> friend class weak_ptr;
    friend struct local_access;

    explicit shared_ptr(shared_ptr_header_block_base *header, ptr_extension_block::uindex_type index) {
        if (header) {
            header->inc_count();
            m_ptr = header->get_ptr_from_index<T>(index);
            m_header = header;
        }
    }

    explicit shared_ptr(counted_ptr rhs) {
        // Creating a shared_ptr bumps my reference count by 1 (we copy the counted_ptr).
        if (rhs.m_header) {
            m_header = rhs.m_header;
            m_header->inc_count();
            m_ptr = m_header->get_ptr_from_index<T>(rhs.m_index);
        }
    }

    explicit operator counted_ptr() const {
        // Creating a counted_ptr bumps my reference count by 1 (we copy the shared_ptr).
        shared_ptr copy(*this);
        counted_ptr result;
        void *ptr = std::exchange(copy.m_ptr, nullptr);
        result.m_header = std::exchange(copy.m_header, nullptr);
        result.m_index = (m_header ? m_header->get_index_from_ptr(ptr) : 0);
        return result;
    }

    void clear() {
        m_ptr = nullptr;
        m_header = nullptr;
    }

public:
    using element_type = T;

    constexpr shared_ptr() noexcept = default;
    constexpr shared_ptr(decltype(nullptr)) noexcept : shared_ptr() {}

    template<class Y> explicit shared_ptr(Y *p) : shared_ptr(p, default_delete<Y>{}) {}

    template<class Y, class D> explicit shared_ptr(Y *p, D d) {
        try {
            m_ptr = p;
            m_header = new shared_ptr_header_separate<Y*,D>(p, d);
        } catch (...) {
            d(p);
            throw;
        }
    }

    template<class D> shared_ptr(decltype(nullptr) p, D d) {
        try {
            m_ptr = p;
            m_header = new shared_ptr_header_separate<decltype(nullptr),D>(p, d);
        } catch (...) {
            d(p);
            throw;
        }
    }

    template<class Y> shared_ptr(const shared_ptr<Y>& r, T *p) noexcept {
        m_ptr = p;
        m_header = r.m_header;
        if (m_header) m_header->inc_count();
    }

    shared_ptr(const shared_ptr& r) noexcept : shared_ptr(r, r.get()) {}
    template<class Y> shared_ptr(const shared_ptr<Y>& r) noexcept : shared_ptr(r.get(), r) {}

    shared_ptr(shared_ptr&& r) noexcept {
        m_ptr = std::exchange(r.m_ptr, nullptr);
        m_header = std::exchange(r.m_header, nullptr);
    }

    template<class Y> shared_ptr(shared_ptr<Y>&& r) noexcept {
        m_ptr = std::exchange(r.m_ptr, nullptr);
        m_header = std::exchange(r.m_header, nullptr);
    }

    template<class Y, class D>
    shared_ptr(unique_ptr<Y, D>&& r):
        m_ptr(r.get()),
        m_header(r.get() ? new shared_ptr_header_separate<Y, D>(r.get(), r.get_deleter()) : nullptr)
    {
        r.release();
    }

    shared_ptr& operator=(const shared_ptr& r) noexcept {
        shared_ptr(r).swap(*this);
        return *this;
    }
    template<class Y> shared_ptr& operator=(const shared_ptr<Y>& r) noexcept {
        shared_ptr(r).swap(*this);
        return *this;
    }
    shared_ptr& operator=(shared_ptr&& r) noexcept {
        shared_ptr(std::move(r)).swap(*this);
        return *this;
    }
    template<class Y> shared_ptr& operator=(shared_ptr<Y>&& r) noexcept {
        shared_ptr(std::move(r)).swap(*this);
        return *this;
    }

    void swap(shared_ptr& r) noexcept {
        std::swap(m_ptr, r.m_ptr);
        std::swap(m_header, r.m_header);
    }

    ~shared_ptr() { if (m_header) m_header->dec_count(); }

    void reset() noexcept { shared_ptr().swap(*this); }
    template<class Y> void reset(Y* p) { shared_ptr(p).swap(*this); }
    template<class Y, class D> void reset(Y* p, D d) { shared_ptr(p, d).swap(*this); }

    T *get() const noexcept { return m_ptr; }
    T& operator*() const noexcept { return *m_ptr; }
    T *operator->() const noexcept { return m_ptr; }
    explicit operator bool() const noexcept { return m_ptr; }

    int use_count() const noexcept { return m_header ? m_header->use_count() : 0; }

    friend inline bool operator==(shared_ptr const& lhs, shared_ptr const& rhs)
        { return lhs.m_ptr == rhs.m_ptr; }
    friend inline bool operator!=(shared_ptr const& lhs, shared_ptr const& rhs)
        { return lhs.m_ptr != rhs.m_ptr; }
};

template<class T>
class weak_ptr {
    T *m_ptr = nullptr;
    shared_ptr_header_block_base *m_header = nullptr;

    void clear() {
        m_header = nullptr;
        m_ptr = nullptr;
    }

public:
    using element_type = T;

    constexpr weak_ptr() noexcept = default;

    template<class Y> weak_ptr(shared_ptr<Y> const& r) noexcept {
        m_ptr = r.m_ptr;
        m_header = r.m_header;
        if (m_header) m_header->inc_weak_count();
    }
    weak_ptr(weak_ptr const& r) noexcept {
        m_ptr = r.m_ptr;
        m_header = r.m_header;
        if (m_header) m_header->inc_weak_count();
    }
    template<class Y> weak_ptr(weak_ptr<Y> const& r) noexcept {
        m_ptr = r.m_ptr;
        m_header = r.m_header;
        if (m_header) m_header->inc_weak_count();
    }
    weak_ptr(weak_ptr&& r) noexcept {
        m_ptr = std::exchange(r.m_ptr, nullptr);
        m_header = std::exchange(r.m_header, nullptr);
    }
    template<class Y> weak_ptr(weak_ptr<Y>&& r) noexcept {
        m_ptr = std::exchange(r.m_ptr, nullptr);
        m_header = std::exchange(r.m_header, nullptr);
    }

    ~weak_ptr() {
        if (m_header) m_header->dec_weak_count();
    }

    weak_ptr& operator=(weak_ptr const& r) noexcept {
        weak_ptr(r).swap(*this);
        return *this;
    }
    template<class Y> weak_ptr& operator=(weak_ptr<Y> const& r) noexcept {
        weak_ptr(r).swap(*this);
        return *this;
    }
    template<class Y> weak_ptr& operator=(shared_ptr<Y> const& r) noexcept {
        weak_ptr(r).swap(*this);
        return *this;
    }
    weak_ptr& operator=(weak_ptr&& r) noexcept {
        weak_ptr(std::move(r)).swap(*this);
        return *this;
    }
    template<class Y> weak_ptr& operator=(weak_ptr<Y>&& r) noexcept {
        weak_ptr(std::move(r)).swap(*this);
        return *this;
    }

    void swap(weak_ptr& r) noexcept {
        std::swap(r.m_header, m_header);
        std::swap(r.m_ptr, m_ptr);
    }

    void reset() noexcept { weak_ptr().swap(*this); }

    int use_count() const noexcept { return m_header ? m_header->use_count() : 0; }
    bool expired() const noexcept { return use_count() == 0; }

    shared_ptr<T> lock() const noexcept {
        shared_ptr<T> result;
        if (m_header && m_header->try_inc_count()) {
            result.m_ptr = m_ptr;
            result.m_header = m_header;
        }
        return result;
    }
};

// ATOMIC STUFF BELOW THIS LINE


struct local_access {
    std::atomic<counted_ptr>& p;
    counted_ptr expected;

    explicit local_access(std::atomic<counted_ptr>& p_) : p(p_) { acquire(p.load()); }
    ~local_access() { release(); }

    void acquire(counted_ptr input) {
        expected = input;
        if (!expected.m_header) return;
        counted_ptr desired;
        do {
            desired = expected;
            desired.m_local_access_count += 1;
        } while (!p.compare_exchange_weak(expected, desired));
        expected = desired;
    }

    void release() {
        if (!expected.m_header) return;
        counted_ptr copy = expected;
        counted_ptr desired;
        do {
            if (expected.m_header != copy.m_header) {
                // Someone has changed the m_header pointer out from under us.
                // Our increment to m_local_access_count has been (or will be) transferred
                // to external_counters. The race between this decrement and that increment
                // is irrelevant because our caller still holds a strong reference to m_header.
                // assert(copy.m_header->use_count() >= 1);
                copy.m_header->dec_external_counters();
                return;
            }
            desired = expected;
            desired.m_local_access_count -= 1;
        } while (!p.compare_exchange_weak(expected, desired));
    }

    void refresh(counted_ptr input) {
        if (input.m_header == expected.m_header) return;
        release();
        acquire(input);
    }
};

template <class T>
class atomic_shared_ptr
{
    mutable std::atomic<counted_ptr> p{counted_ptr{}};

    template<class U> friend class atomic_shared_ptr;
public:
    bool is_lock_free() const noexcept { return p.is_lock_free(); }

    shared_ptr<T> load() const noexcept {
        local_access guard(p);
        return shared_ptr<T>(guard.expected);
    }

    shared_ptr<T> exchange(shared_ptr<T> rhs) noexcept {
        counted_ptr desired(std::move(rhs));
        counted_ptr old = p.exchange(desired);
        shared_ptr<T> result(old);
        old.destroy();  // manually destroy the POD counted_ptr "old"
        return result;
    }

    bool compare_exchange_weak(shared_ptr<T>& expected, shared_ptr<T> desired) noexcept
    {
        local_access guard(p);
        if (guard.expected.m_header != expected.m_header) {
            expected = shared_ptr<T>(guard.expected);
            return false;
        }
        counted_ptr oldval = guard.expected;
        counted_ptr newval(std::move(desired));
        if (p.compare_exchange_weak(oldval, newval)) {
            // The swap succeeded. p used to contain oldval and now contains newval.
            oldval.destroy();  // manually destroy the POD counted_ptr "oldval"
            return true;
        } else {
            // The swap failed. p contains oldval.
            guard.refresh(oldval);
            expected = shared_ptr<T>(guard.expected);
            return false;
        }
    }

    bool compare_exchange_strong(shared_ptr<T>& expected, shared_ptr<T> desired) noexcept {
        shared_ptr<T> copy = expected;
        do {
            if (compare_exchange_weak(expected, desired))
                return true;
        } while (expected == copy);
        return false;
    }

    atomic_shared_ptr() noexcept = default;
    atomic_shared_ptr(shared_ptr<T> rhs) noexcept { this->exchange(std::move(rhs)); }
    atomic_shared_ptr(const atomic_shared_ptr&) = delete;
    atomic_shared_ptr& operator=(const atomic_shared_ptr&) = delete;
    atomic_shared_ptr& operator=(shared_ptr<T> rhs) noexcept { this->exchange(std::move(rhs)); return *this; }
    operator shared_ptr<T>() const noexcept { return load(); }
    void store(shared_ptr<T> rhs) noexcept { this->exchange(std::move(rhs)); }

    ~atomic_shared_ptr() {
        counted_ptr old = p.load();
        // assert(old.m_local_access_count == 0);
        old.destroy();
    }
};

} // namespace scratch::jss
