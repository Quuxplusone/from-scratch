#pragma once

#include "scratch/bits/concurrency/linux-futex.h"

#include <atomic>
#include <utility>

namespace scratch {

class once_flag {
    static constexpr int UNDONE = 0;
    static constexpr int IN_PROGRESS = 1;
    static constexpr int DONE = 2;

    std::atomic<int> m_futex;
public:
    constexpr once_flag() noexcept : m_futex(UNDONE) {}

    template<class F, class... Args>
    void call_once(F&& f, Args&&... args)
    {
        int x = UNDONE;
        while (!m_futex.compare_exchange_weak(x, IN_PROGRESS)) {
            if (x == DONE) return;
            futex_wait(&m_futex, IN_PROGRESS);
            x = UNDONE;
        }
        try {
            std::forward<F>(f)(std::forward<Args>(args)...);
        } catch (...) {
            m_futex = UNDONE;
            futex_wake_one(&m_futex);
            throw;
        }
        m_futex = DONE;
        futex_wake_all(&m_futex);
    }
};

template<class F, class... Args>
void call_once(once_flag& flag, F&& f, Args&&... args)
{
    flag.call_once(std::forward<F>(f), std::forward<Args>(args)...);
}

} // namespace scratch
