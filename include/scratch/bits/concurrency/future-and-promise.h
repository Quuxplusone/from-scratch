#pragma once

#include "scratch/bits/concurrency/condition-variable.h"
#include "scratch/bits/concurrency/lock-guard.h"
#include "scratch/bits/concurrency/mutex.h"
#include "scratch/bits/error-code/future-category.h"
#include "scratch/bits/optional/optional.h"
#include "scratch/bits/smart-ptrs/shared-ptr.h"
#include "scratch/bits/smart-ptrs/make-shared.h"
#include "scratch/bits/stdexcept/future-error.h"
#include "scratch/bits/stdexcept/make-exception-ptr.h"

#include <atomic>
#include <exception>
#include <utility>

namespace scratch::detail {

template<class T>
class future_shared_state
{
    mutable mutex m_mtx;
    condition_variable m_cv;
    std::atomic<bool> m_ready{false};
    optional<T> m_value;
    std::exception_ptr m_exception;
public:
    bool ready() const { return m_ready; }

    template<class... Args>
    void set_value(Args&&... args) {
        if (m_ready) {
            throw future_error(future_errc::promise_already_satisfied);
        }
        m_value.emplace(std::forward<Args>(args)...);
        set_ready();
    }

    void set_exception(std::exception_ptr ex) {
        if (m_ready) {
            throw future_error(future_errc::promise_already_satisfied);
        }
        m_exception = std::move(ex);
        set_ready();
    }

    void set_ready() {
        lock_guard lk(m_mtx);
        m_ready = true;
        m_cv.notify_all();
    }

    void wait() {
        unique_lock lk(m_mtx);
        while (!m_ready) {
            m_cv.wait(lk);
        }
    }

    decltype(auto) get_value_assuming_ready() {
        if (m_value) {
            return *m_value;
        } else {
            std::rethrow_exception(m_exception);
        }
    }
};

} // namespace scratch::detail

namespace scratch {

template<class T> class future;
template<class T> class shared_future;

template<class T>
class promise {
    shared_ptr<detail::future_shared_state<T>> m_ptr;

public:
    promise() { m_ptr = make_shared<detail::future_shared_state<T>>(); }

    promise(const promise&) = delete;
    promise(promise&&) noexcept = default;
    promise& operator=(promise&& rhs) noexcept { promise(std::move(rhs)).swap(*this); return *this; }
    promise& operator=(const promise&) = delete;

    void swap(promise& rhs) { m_ptr.swap(rhs.m_ptr); }

    ~promise() {
        if (valid() && !ready()) {
            set_exception(make_exception_ptr(future_error(future_errc::broken_promise)));
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

    future<T> get_future() {
        if (!valid()) throw future_error(future_errc::no_state);
        if (m_ptr.use_count() != 1) throw future_error(future_errc::future_already_retrieved);
        return future<T>(m_ptr);
    }
};

template<class T>
class future {
    shared_ptr<detail::future_shared_state<T>> m_ptr;

    future(shared_ptr<detail::future_shared_state<T>> p) : m_ptr(std::move(p)) {}

    friend class promise<T>;
public:
    future() noexcept = default;
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
        auto sptr = std::move(m_ptr);
        return std::move(sptr->get_value_assuming_ready());
    }
};

template<class T>
class shared_future {
    shared_ptr<detail::future_shared_state<T>> m_ptr;

    shared_future(shared_ptr<detail::future_shared_state<T>> p) : m_ptr(std::move(p)) {}

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
        return m_ptr->get_value_assuming_ready();
    }
};

} // namespace scratch
