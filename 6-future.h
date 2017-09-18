#pragma once

#include <condition_variable>
#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

namespace my::detail {

template<class T>
class shared_state
{
    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::optional<T> m_value;
    std::exception_ptr m_exception;
public:
    template<class... Args>
    void set_value(Args&&... args) {
        std::lock_guard lk(m_mtx);
        if (m_value || m_exception) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }
        m_value.emplace(std::forward<Args>(args)...);
        m_cv.notify_all();
    }

    void set_exception(std::exception_ptr ex) {
        std::lock_guard lk(m_mtx);
        if (m_value || m_exception) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }
        m_exception = std::move(ex);
        m_cv.notify_all();
    }

    void wait() {
        std::unique_lock lk(m_mtx);
        while (!(m_value || m_exception)) {
            m_cv.wait(lk);
        }
    }

    decltype(auto) get_value_unlocked() {
        if (m_value) {
            return *m_value;
        } else {
            std::rethrow_exception(m_exception);
        }
    }
};

} // namespace my::detail

namespace my {

template<class T> class future;
template<class T> class shared_future;

template<class T>
class promise {
    std::shared_ptr<detail::shared_state<T>> m_ptr;

public:
    promise() { m_ptr = std::make_shared<detail::shared_state<T>>(); }

    promise(const promise&) = delete;
    promise(promise&&) noexcept = default;
    promise& operator=(promise&&) noexcept = default;
    promise& operator=(const promise&) = delete;

    void swap(promise& rhs) { m_ptr.swap(rhs.m_ptr); }

    ~promise() {
        // if (valid() && !ready()) {
        //     this->set_exception(std::make_exception_ptr(std::future_error(std::future_errc::broken_promise)));
        // }
    }

    bool valid() const { return m_ptr != nullptr; }

    void set_value(T t) {
        if (!valid()) throw std::future_error(std::future_errc::no_state);
        m_ptr->set_value(std::move(t));
    }

    void set_exception(std::exception_ptr ex) {
        if (!valid()) throw std::future_error(std::future_errc::no_state);
        m_ptr->set_exception(std::move(ex));
    }

    future<T> get_future() { return future<T>(m_ptr); }
};

template<class T>
class future {
    std::shared_ptr<detail::shared_state<T>> m_ptr;

    future(std::shared_ptr<detail::shared_state<T>> p) : m_ptr(std::move(p)) {}

    friend class promise<T>;
public:
    future(const future&) = delete;
    future(future&&) noexcept = default;
    future& operator=(future&&) noexcept = default;
    future& operator=(const future&) = delete;

    void swap(future& rhs) { m_ptr.swap(rhs.m_ptr); }

    // shared_future<T> share() noexcept;

    bool valid() const { return m_ptr != nullptr; }

    void wait() {
        if (!valid()) throw std::future_error(std::future_errc::no_state);
        m_ptr->wait();
    }

    T get() {
        this->wait();
        return std::move(m_ptr->get_value_unlocked());
    }
};

template<class T>
void swap(future<T>& a, future<T>& b) {
    a.swap(b);
}

#include "shared-future.h"

} // namespace my

