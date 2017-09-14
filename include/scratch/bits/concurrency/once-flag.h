#pragma once

#include "scratch/bits/concurrency/condition-variable.h"
#include "scratch/bits/concurrency/mutex.h"
#include "scratch/bits/concurrency/unique-lock.h"

#include <utility>

namespace scratch {

class once_flag {
    static constexpr int UNDONE = 0;
    static constexpr int IN_PROGRESS = 1;
    static constexpr int DONE = 2;

    mutex m_mtx;
    condition_variable m_cv;
    int m_flag;
public:
    constexpr once_flag() noexcept : m_flag(UNDONE) {}

    template<class F, class... Args>
    void call_once(F&& f, Args&&... args)
    {
        unique_lock<mutex> lk(m_mtx);
        while (m_flag == IN_PROGRESS) {
            m_cv.wait(lk);
        }
        if (m_flag == UNDONE) {
            m_flag = IN_PROGRESS;
            lk.unlock();
            try {
                std::forward<F>(f)(std::forward<Args>(args)...);
            } catch (...) {
                lk.lock();
                m_flag = UNDONE;
                m_cv.notify_one();
                throw;
            }
            lk.lock();
            m_flag = DONE;
            m_cv.notify_all();
        }
    }
};

template<class F, class... Args>
void call_once(once_flag& flag, F&& f, Args&&... args)
{
    flag.call_once(std::forward<F>(f), std::forward<Args>(args)...);
}

} // namespace scratch
