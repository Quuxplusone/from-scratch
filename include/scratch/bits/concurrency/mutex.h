#pragma once

#include "scratch/bits/concurrency/linux-futex.h"

#include <atomic>

namespace scratch {

class mutex
{
    static constexpr int UNLOCKED = 0;
    static constexpr int LOCKED = 1;
    std::atomic<int> m_futex;
public:
    constexpr mutex() noexcept : m_futex(UNLOCKED) {}

    bool try_lock() {
        return (m_futex.exchange(LOCKED) == UNLOCKED);
    }

    void lock() {
        int old_value = m_futex.exchange(LOCKED);
        while (old_value != UNLOCKED) {
            futex_wait(&m_futex, LOCKED);
            old_value = m_futex.exchange(LOCKED);
        }
    }

    void unlock() {
        m_futex = UNLOCKED;
        futex_wake_one(&m_futex);
    }
};

} // namespace scratch
