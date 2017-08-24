#pragma once

#include "scratch/bits/smart-ptrs/default-delete.h"
#include "scratch/bits/smart-ptrs/smart-ptr-base.h"
#include "scratch/bits/smart-ptrs/unique-ptr.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-fooible.h"

#include <atomic>
#include <utility>

namespace scratch {

template<typename T>
class atomic_unique_ptr
{
public:
    using element_type = typename detail::smart_ptr_base<T>::element_type;
    using pointer = element_type*;
    using deleter_type = default_delete<T>;

    static constexpr bool is_always_lock_free = true;
    static constexpr bool is_lock_free() noexcept { return true; }

private:
    std::atomic<element_type *> m_ptr;

public:
    constexpr atomic_unique_ptr() noexcept : m_ptr(nullptr) {}
    constexpr atomic_unique_ptr(decltype(nullptr)) noexcept : m_ptr(nullptr) {}
    constexpr atomic_unique_ptr(unique_ptr<T> up) noexcept : atomic_unique_ptr(up.release()) {}
    constexpr atomic_unique_ptr(element_type *p) noexcept : m_ptr(p) {}

    atomic_unique_ptr(atomic_unique_ptr&&) = delete;
    atomic_unique_ptr(const atomic_unique_ptr&) = delete;
    atomic_unique_ptr& operator=(atomic_unique_ptr&&) = delete;
    atomic_unique_ptr& operator=(const atomic_unique_ptr&) = delete;

    ~atomic_unique_ptr() { reset(); }

    template<typename U, bool_if_t<is_convertible_v<U*, T*>> = true>
    atomic_unique_ptr& operator=(unique_ptr<U> rhs) noexcept {
        store(unique_ptr<T>(std::move(rhs)));
        return *this;
    }

    atomic_unique_ptr& operator=(decltype(nullptr)) noexcept {
        store(unique_ptr<T>(nullptr));
        return *this;
    }

    T *get() const noexcept {
        return m_ptr.load();
    }
    unique_ptr<T> exchange(unique_ptr<T> rhs) noexcept {
        T *p = rhs.release();
        T *old = m_ptr.exchange(p);
        return unique_ptr<T>(old);
    }

    // compare_exchange_weak has no sane signature, so omit it.

    unique_ptr<T> load() noexcept {
        return exchange(nullptr);
    }
    void store(unique_ptr<T> rhs) noexcept {
        exchange(std::move(rhs));
    }

    element_type *release() noexcept {
        unique_ptr<T> u = exchange(nullptr);
        return u.release();
    }
    void reset(element_type *p = nullptr) noexcept {
        store(unique_ptr<T>(p));
    }

    constexpr explicit operator bool() const noexcept { return get() != nullptr; }
    constexpr auto& operator*() const noexcept { return *get(); }
    constexpr T* operator->() const noexcept { return get(); }
};

} // namespace scratch
