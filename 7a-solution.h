#pragma once

#include "unique-function.h"
#include <condition_variable>
#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

namespace my {

template<class T>
class future_shared_state
{
    mutable std::mutex m_mtx;
    std::condition_variable m_cv;
    std::atomic<bool> m_ready{false};
    std::optional<T> m_value;
    std::exception_ptr m_exception;
    unique_function<void()> m_then;
public:
    bool ready() const {
        return m_ready;
    }

    template<class... Args>
    void set_value(Args&&... args) {
        if (m_ready) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }
        m_value.emplace(std::forward<Args>(args)...);
        set_ready();
    }

    void set_exception(std::exception_ptr ex) {
        if (m_ready) {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }
        m_exception = std::move(ex);
        set_ready();
    }

    void set_ready() {
        unique_function<void()> continuation;
        if (true) {
            std::lock_guard lk(m_mtx);
            m_ready = true;
            m_then.swap(continuation);
            m_cv.notify_all();
        }
        if (continuation) {
            continuation();
        }
    }

    void set_continuation(unique_function<void()> continuation) {
        bool just_run_it = false;
        if (true) {
            std::lock_guard lk(m_mtx);
            just_run_it = m_ready;
            if (!just_run_it) {
                m_then.swap(continuation);
            }
        }
        if (just_run_it) {
            continuation();
        }
    }

    void wait() {
        std::unique_lock lk(m_mtx);
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

template<class T> class future;
template<class T> class shared_future;

template<class T>
class promise {
    std::shared_ptr<future_shared_state<T>> m_ptr;

public:
    promise() { m_ptr = std::make_shared<future_shared_state<T>>(); }

    promise(const promise&) = delete;
    promise(promise&&) noexcept = default;
    promise& operator=(promise&& rhs) noexcept { promise(std::move(rhs)).swap(*this); return *this; }
    promise& operator=(const promise&) = delete;

    void swap(promise& rhs) { m_ptr.swap(rhs.m_ptr); }

    ~promise() {
        if (valid() && !ready()) {
            set_exception(std::make_exception_ptr(std::future_error(std::future_errc::broken_promise)));
        }
    }

    bool valid() const { return m_ptr != nullptr; }
    bool ready() const { return valid() && m_ptr->ready(); }

    void set_value(T t) {
        if (!valid()) throw std::future_error(std::future_errc::no_state);
        m_ptr->set_value(std::move(t));
    }

    void set_exception(std::exception_ptr ex) {
        if (!valid()) throw std::future_error(std::future_errc::no_state);
        m_ptr->set_exception(std::move(ex));
    }

    future<T> get_future() {
        if (!valid()) throw std::future_error(std::future_errc::no_state);
        if (m_ptr.use_count() != 1) throw std::future_error(std::future_errc::future_already_retrieved);
        return future<T>(m_ptr);
    }
};

template<class T>
class future {
    std::shared_ptr<future_shared_state<T>> m_ptr;

    future(std::shared_ptr<future_shared_state<T>> p) : m_ptr(std::move(p)) {}

    friend class promise<T>;
public:
    future(const future&) = delete;
    future(future&&) noexcept = default;
    future& operator=(future&&) noexcept = default;
    future& operator=(const future&) = delete;

    void swap(future& rhs) { m_ptr.swap(rhs.m_ptr); }

    bool valid() const { return m_ptr != nullptr; }
    bool ready() const { return valid() && m_ptr->ready(); }

    void wait() {
        if (!valid()) throw std::future_error(std::future_errc::no_state);
        m_ptr->wait();
    }

    T get() {
        wait();
        auto sptr = std::move(m_ptr);
        return std::move(sptr->get_value_assuming_ready());
    }

    template<class F>
    auto then(F func) {
        if (!valid()) throw std::future_error(std::future_errc::no_state);
        using R = decltype(func(std::move(*this)));
        promise<R> p;
        future<R> result = p.get_future();
        unique_function<void()> continuation = [p = std::move(p), func = std::move(func), sptr = m_ptr]() mutable {
            try {
                future self(std::move(sptr));
                p.set_value(func(std::move(self)));
            } catch (...) {
                p.set_exception(std::current_exception());
            }
        };
        auto sptr = std::move(m_ptr);
        sptr->set_continuation(std::move(continuation));
        return result;
    }

    template<class F>
    auto next(F func) {
        return this->then([func = std::move(func)](auto self) {
            return func(self.get());
        });
    }

    template<class F>
    future recover(F func) {
        return this->then([func = std::move(func)](auto self) {
            try {
                return self.get();
            } catch (...) {
                return func(std::current_exception());
            }
        });
    }

    future fallback_to(T value) {
        return this->then([value = std::move(value)](auto self) {
            try {
                return self.get();
            } catch (...) {
                return value;
            }
        });
    }
};

} // namespace my
