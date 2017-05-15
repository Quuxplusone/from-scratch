#pragma once

#include "scratch/bits/concurrency/condition-variable-any.h"
#include "scratch/bits/concurrency/mutex.h"
#include "scratch/bits/concurrency/unique-lock.h"

#include <utility>

namespace scratch {

class condition_variable
{
    condition_variable_any m_cv;
public:
    void notify_one() noexcept {
        m_cv.notify_one();
    }

    void notify_all() noexcept {
        m_cv.notify_all();
    }

    void wait(unique_lock<mutex>& lk) {
        m_cv.wait(lk);
    }

    template<typename Predicate>
    void wait(unique_lock<mutex>& lk, Predicate p) {
        m_cv.wait(lk, std::move(p));
    }
};

} // namespace scratch
