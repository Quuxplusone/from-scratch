#pragma once

#include "scratch/bits/concurrency/lock-guard.h"
#include "scratch/bits/concurrency/mutex.h"
#include "scratch/bits/smart-ptrs/default-delete.h"
#include "scratch/bits/smart-ptrs/shared-ptr.h"
#include "scratch/bits/smart-ptrs/smart-ptr-base.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-fooible.h"

#include <utility>

namespace scratch {

template<typename T>
class atomic_shared_ptr
{
    shared_ptr<T> m_ptr = nullptr;
    mutable mutex m_mtx;

    mutex& get_mutex() const { return m_mtx; }

    static constexpr bool is_always_lock_free = false;
    static constexpr bool is_lock_free() noexcept { return false; }

public:
    using element_type = typename detail::smart_ptr_base<T>::element_type;

    constexpr atomic_shared_ptr() noexcept = default;

    template<class... Args, class = enable_if_t<is_constructible_v<shared_ptr<T>, Args&&...>>>
    constexpr atomic_shared_ptr(Args&&... args) noexcept : m_ptr(std::forward<Args>(args)...) {}

    atomic_shared_ptr(atomic_shared_ptr&&) = delete;
    atomic_shared_ptr(const atomic_shared_ptr&) = delete;
    atomic_shared_ptr& operator=(atomic_shared_ptr&&) = delete;
    atomic_shared_ptr& operator=(const atomic_shared_ptr&) = delete;

    template<typename U, bool_if_t<is_convertible_v<U*, T*>> = true>
    atomic_shared_ptr& operator=(shared_ptr<U> rhs) noexcept {
        store(shared_ptr<T>(std::move(rhs)));
        return *this;
    }

    atomic_shared_ptr& operator=(decltype(nullptr)) noexcept {
        reset();
        return *this;
    }

    operator shared_ptr<T>() const { return load(); }

    T *get() const noexcept {
        lock_guard<mutex> lk(get_mutex());
        return m_ptr.get();
    }
    shared_ptr<T> exchange(shared_ptr<T> rhs) noexcept {
        shared_ptr<T> copy(std::move(rhs));
        if (lock_guard<mutex> lk(get_mutex()); true) {
            copy.swap(m_ptr);
        }
        return copy;
    }

    bool compare_exchange_weak(shared_ptr<T>& expected, shared_ptr<T>&& desired) noexcept {
        shared_ptr<T> copy;
        lock_guard<mutex> lk(get_mutex());
        if (m_ptr == expected) {
            copy = std::move(desired);
            copy.swap(m_ptr);
            return true;
        } else {
            copy = m_ptr;
            copy.swap(expected);
            return false;
        }
    }
    bool compare_exchange_weak(shared_ptr<T>& expected, const shared_ptr<T>& desired) noexcept {
        shared_ptr<T> copy;
        lock_guard<mutex> lk(get_mutex());
        if (m_ptr == expected) {
            copy = desired;
            copy.swap(m_ptr);
            return true;
        } else {
            copy = m_ptr;
            copy.swap(expected);
            return false;
        }
    }
    bool compare_exchange_strong(shared_ptr<T>& expected, shared_ptr<T>&& desired) noexcept {
        return compare_exchange_weak(expected, std::move(desired));
    }
    bool compare_exchange_strong(shared_ptr<T>& expected, const shared_ptr<T>& desired) noexcept {
        return compare_exchange_weak(expected, desired);
    }

    shared_ptr<T> load() const noexcept {
        lock_guard<mutex> lk(get_mutex());
        return m_ptr;
    }
    void store(shared_ptr<T> rhs) noexcept {
        exchange(std::move(rhs));
    }

    void reset() noexcept {
        shared_ptr<T> copy;
        lock_guard<mutex> lk(get_mutex());
        copy.swap(m_ptr);
    }
    template<class Y> void reset(Y *ptr) {
        shared_ptr<T> copy(shared_ptr<Y>(ptr, default_delete<Y>()));
        lock_guard<mutex> lk(get_mutex());
        copy.swap(m_ptr);
    }
    template<class Y, class Deleter> void reset(Y *ptr, Deleter d) {
        shared_ptr<Y> copy(shared_ptr<Y>(ptr, std::move(d)));
        lock_guard<mutex> lk(get_mutex());
        copy.swap(m_ptr);
    }

    long use_count() const noexcept {
        lock_guard<mutex> lk(get_mutex());
        return m_ptr.use_count();
    }

    constexpr explicit operator bool() const noexcept { return get() != nullptr; }
    constexpr auto& operator*() const noexcept { return *get(); }
    constexpr T* operator->() const noexcept { return get(); }
};

template<class T> bool operator==(const atomic_shared_ptr<T>& a, decltype(nullptr)) noexcept { return a.get() == nullptr; }
template<class T> bool operator!=(const atomic_shared_ptr<T>& a, decltype(nullptr)) noexcept { return a.get() != nullptr; }
template<class T> bool operator==(decltype(nullptr), const atomic_shared_ptr<T>& a) noexcept { return a.get() == nullptr; }
template<class T> bool operator!=(decltype(nullptr), const atomic_shared_ptr<T>& a) noexcept { return a.get() != nullptr; }

template<class T, class U> bool operator==(const atomic_shared_ptr<T>& a, const shared_ptr<U>& b) noexcept { return a.get() == b.get(); }
template<class T, class U> bool operator!=(const atomic_shared_ptr<T>& a, const shared_ptr<U>& b) noexcept { return a.get() != b.get(); }
template<class T, class U> bool operator==(const shared_ptr<T>& a, const atomic_shared_ptr<U>& b) noexcept { return a.get() == b.get(); }
template<class T, class U> bool operator!=(const shared_ptr<T>& a, const atomic_shared_ptr<U>& b) noexcept { return a.get() != b.get(); }

} // namespace scratch
