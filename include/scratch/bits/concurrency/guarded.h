#pragma once

#include "scratch/bits/concurrency/mutex.h"
#include "scratch/bits/concurrency/unique-lock.h"
#include "scratch/bits/stdexcept/bad-guarded-access.h"

#include <utility>

namespace scratch {

template<class T, class M = mutex>
class guarded {
    M m_mutex;
    T m_data;
public:
    class handle {
        unique_lock<M> m_lock;
        T *m_data;
    public:
        handle(unique_lock<M> lk, T *p) noexcept : m_lock(std::move(lk)), m_data(p) {}

        bool owns_lock() const noexcept { return m_lock.owns_lock(); }

        T *operator->() const {
            if (!owns_lock()) throw bad_guarded_access();
            return m_data;
        }

        T& operator*() const {
            if (!owns_lock()) throw bad_guarded_access();
            return *m_data;
        }

        unique_lock<M> unique_lock() noexcept {
            return std::move(m_lock);
        }
    };

    handle lock() {
        unique_lock<M> lk(m_mutex);
        return handle(std::move(lk), &m_data);
    }

    handle lock(unique_lock<M> lk) {
        if (lk.mutex() != &m_mutex) throw bad_guarded_access();
        return handle(std::move(lk), &m_data);
    }

    handle try_lock() {
        unique_lock<M> lk(m_mutex, scratch::try_to_lock);
        return handle(std::move(lk), &m_data);
    }
};

} // namespace scratch
