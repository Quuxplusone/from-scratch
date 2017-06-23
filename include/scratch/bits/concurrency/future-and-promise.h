#pragma once

#include "scratch/bits/concurrency/condition-variable.h"
#include "scratch/bits/concurrency/lock-guard.h"
#include "scratch/bits/concurrency/mutex.h"
#include "scratch/bits/smart-ptrs/shared-ptr.h"
#include "scratch/bits/smart-ptrs/make-shared.h"
#include "scratch/bits/optional/optional.h"

#include <exception>
#include <utility>

namespace scratch::detail {

template<class T>
class future_shared_state
{
    mutex m_mtx;
    condition_variable m_cv;
    optional<T> m_value;
    exception_ptr m_exception;
public:
    bool ready() const { lock_guard lk(m_mtx); return m_value || m_exception; }

    template<class... Args>
    void set_value(Args&&... args) {
        lock_guard lk(m_mtx);
        if (m_value || m_exception) {
            throw future_error(future_errc::promise_already_satisfied);
        }
        m_value.emplace(std::forward<Args>(args)...);
        m_cv.notify_all();
    }

    void set_exception(exception_ptr ex) {
        lock_guard lk(m_mtx);
        if (m_value || m_exception) {
            throw future_error(future_errc::promise_already_satisfied);
        }
        m_exception = std::move(ex);
        m_cv.notify_all();
    }

    void wait() {
        unique_lock lk(m_mtx);
        while (!(m_value || m_exception)) {
            m_cv.wait(lk);
        }
    }
};

} // namespace scratch::detail

namespace scratch {

template<class T>
class promise {
    shared_ptr<future_shared_state<T>> m_ptr;

public:
    promise() { m_ptr = make_shared<future_shared_state<T>>(); }

    promise(const promise&) = delete;
    promise(promise&&) noexcept = default;
    promise& operator=(promise&&) noexcept = default;
    promise& operator=(const promise&) = delete;

    void swap(promise& rhs) { m_ptr.swap(rhs.m_ptr); }

    ~promise() {
        if (valid() && !ready()) {
            set_exception(future_error(future_errc::broken_promise));
        }
    }

    bool valid() const { return m_ptr != nullptr; }
    bool ready() const { return valid() && m_ptr->ready(); }

    void set_value(T t) {
        if (!valid()) throw future_error(future_errc::no_state);
        m_ptr->set_value(std::move(t));
    }

    void set_exception(std::exception_ptr ex) {
        if (!valid()) throw future_error(future_errc::no_state);
        m_ptr->set_exception(std::move(ex));
    }

    future<T> get_future() { return future<T>(m_ptr); }
};

template<class T>
class future {
    shared_ptr<future_shared_state<T>> m_ptr;

    future(shared_ptr<future_shared_state<T>> p) : m_ptr(std::move(p)) {}

    friend class promise<T>;
public:
    future(const future&) = delete;
    future(future&&) noexcept = default;
    future& operator=(future&&) noexcept = default;
    future& operator=(const future&) = delete;

    void swap(future& rhs) { m_ptr.swap(rhs.m_ptr); }

    shared_future<T> share() noexcept {
        return shared_future<T>(std::move(m_ptr));
    }

    bool valid() const { return m_ptr != nullptr; }
    bool ready() const { return valid() && m_ptr->ready(); }

    void wait() {
        if (!valid()) throw future_error(future_errc::no_state);
        m_ptr->wait();
    }

    T get() {
        wait();
        if (m_ptr->m_value) {
            return std::move(*m_ptr->m_value);
        } else {
            std::rethrow_exception(m_ptr->m_exception);
        }
    }
};

template<class T>
class shared_future {
    shared_ptr<future_shared_state<T>> m_ptr;

    shared_future(shared_ptr<future_shared_state<T>> p) : m_ptr(std::move(p)) {}

    friend class promise<T>;
public:
    void swap(shared_future& rhs) { m_ptr.swap(rhs.m_ptr); }

    bool valid() const { return m_ptr != nullptr; }
    bool ready() const { return valid() && m_ptr->ready(); }

    void wait() {
        if (!valid()) throw future_error(future_errc::no_state);
        m_ptr->wait();
    }

    const T& get() {
        wait();
        if (m_ptr->m_value) {
            return *m_ptr->m_value;
        } else {
            std::rethrow_exception(m_ptr->m_exception);
        }
    }
};

} // namespace scratch
