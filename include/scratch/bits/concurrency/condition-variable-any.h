#pragma once

#include "scratch/bits/concurrency/linux-futex.h"

#include <atomic>

namespace scratch {

class condition_variable_any
{
    std::atomic<int> m_value = 0;
public:
    void notify_one() noexcept {
        m_value |= 1;
        futex_wake_one(&m_value);
    }

    void notify_all() noexcept {
        m_value |= 1;
        futex_wake_all(&m_value);
    }

    template<typename Lock>
    void wait(Lock& lk) {
        int value = m_value.load();
        // Get ourselves a "sequence number" with a 0 in the low-order bits.
        if (value & 1) {
            if (!m_value.compare_exchange_strong(value, value + 1)) {
                if (value & 1) {
                    // Someone JUST did a notify! Might as well wake up.
                    return;
                }
            }
        }
        // Now we've got a sequence number.
        // If m_value ever changes away from that number again,
        // it can't be due to wait(), so it must be time for us to wake up.
        lk.unlock();
        futex_wait(&m_value, value);
        lk.lock();
    }

    template<typename Lock, typename Predicate>
    void wait(Lock& lk, Predicate p) {
        while (!p()) {
            this->wait(lk);
        }
    }
};

} // namespace scratch
