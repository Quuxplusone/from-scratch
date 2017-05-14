#pragma once

#include "scratch/bits/concurrency/linux-futex.h"

#include <atomic>

namespace scratch {

class mutex
{
    static constexpr int UNLOCKED = 0;
    static constexpr int LOCKED_WITHOUT_WAITERS = 1;
    static constexpr int LOCKED_WITH_WAITERS = 2;
    std::atomic<int> m_futex;
public:
    constexpr mutex() noexcept : m_futex(UNLOCKED) {}

    bool try_lock() {
        int x = UNLOCKED;
        if (m_futex.compare_exchange_weak(x, LOCKED_WITHOUT_WAITERS)) {
            // the uncontended case
            return true;
        } else {
            return false;
        }
    }

    void lock() {
        int x = UNLOCKED;
        if (m_futex.compare_exchange_weak(x, LOCKED_WITHOUT_WAITERS)) {
            // the uncontended case
        } else {
            if (x == LOCKED_WITH_WAITERS) {
                futex_wait(&m_futex, LOCKED_WITH_WAITERS);
            }
            while (m_futex.exchange(LOCKED_WITH_WAITERS) != UNLOCKED) {
                futex_wait(&m_futex, LOCKED_WITH_WAITERS);
            }
        }
    }

    void unlock() {
        int locked_state = --m_futex;
        if (new_state == UNLOCKED) {
            // the uncontended case
        } else {
            m_futex = UNLOCKED;
            futex_wake_one(&m_futex);
        }
    }
};

} // namespace scratch
