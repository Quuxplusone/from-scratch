#pragma once

#include "scratch/bits/concurrency/tag-types.h"

namespace scratch {

template<class Mutex>
class lock_guard
{
    Mutex& m_mtx;

public:
    using mutex_type = Mutex;

    explicit lock_guard(mutex_type& m) : m_mtx(m) {
        m_mtx.lock();
    }

    lock_guard(mutex_type& m, adopt_lock_t) : m_mtx(m) {}

    ~lock_guard() {
        m_mtx.unlock();
    }
};

} // namespace scratch
