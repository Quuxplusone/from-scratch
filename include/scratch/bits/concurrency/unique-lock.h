#pragma once

#include "scratch/bits/concurrency/tag-types.h"

#include <utility>

namespace scratch {

template<class Mutex>
class unique_lock
{
    Mutex *m_mtx;
    bool m_locked;

public:
    using mutex_type = Mutex;

    unique_lock() noexcept : m_mtx(nullptr), m_locked(false) {}

    unique_lock(unique_lock&& rhs) noexcept {
        m_mtx = std::exchange(rhs.m_mtx, nullptr);
        m_locked = std::exchange(rhs.m_locked, false);
    }

    unique_lock& operator=(unique_lock&& rhs) {
        if (m_locked) {
            m_mtx->unlock();
        }
        m_mtx = std::exchange(rhs.m_mtx, nullptr);
        m_locked = std::exchange(rhs.m_locked, false);
    }

    void swap(unique_lock& rhs) noexcept {
        std::swap(m_mtx, rhs.m_mtx);
        std::swap(m_locked, rhs.m_locked);
    }

    explicit unique_lock(mutex_type& m) : m_mtx(&m), m_locked(true) {
        m_mtx->lock();
    }

    unique_lock(mutex_type& m, adopt_lock_t) : m_mtx(&m), m_locked(true) {}
    unique_lock(mutex_type& m, defer_lock_t) : m_mtx(&m), m_locked(false) {}
    unique_lock(mutex_type& m, try_to_lock_t) : m_mtx(&m), m_locked(m.try_lock()) {}

    ~unique_lock() {
        if (m_locked) {
            m_mtx->unlock();
        }
    }

    void lock() { m_mtx->lock(); m_locked = true; }
    void unlock() { m_mtx->unlock(); m_locked = false; }

    mutex_type *mutex() const noexcept { return m_mtx; }
    bool owns_lock() const noexcept { return m_locked; }
    explicit operator bool() const noexcept { return owns_lock(); }

    mutex_type *release() const noexcept {
        m_locked = false;
        return std::exchange(m_mtx, nullptr);
    }
};

} // namespace scratch
