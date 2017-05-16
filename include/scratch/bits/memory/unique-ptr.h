#pragma once

#include "scratch/bits/memory/default-delete.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-fooible.h"
#include "scratch/bits/utility/compressed-pair.h"

#include <utility>

namespace scratch {

template<typename T, typename Deleter = default_delete<T>>
class unique_ptr
{
    compressed_pair<T *, Deleter> m_pair;

public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    constexpr unique_ptr() noexcept : m_pair(nullptr, {}) {}
    constexpr unique_ptr(T *p) noexcept : m_pair(p, {}) {}
    constexpr unique_ptr(T *p, Deleter d) noexcept : m_pair(p, std::move(d)) {}

    unique_ptr(unique_ptr&& rhs) noexcept : unique_ptr() {
        this->reset(rhs.release());
        get_deleter() = std::move(rhs.get_deleter());
    }

    unique_ptr& operator=(unique_ptr&& rhs) noexcept {
        reset(rhs.release());
        get_deleter() = std::move(rhs.get_deleter());
        return *this;
    }

    template<typename U, typename E,
        typename = enable_if_t<!is_array_v<U> && is_convertible_v<U*, T*> && is_assignable_v<Deleter&, E&&>>>
    unique_ptr& operator=(unique_ptr<U,E>&& rhs) noexcept {
        reset(static_cast<T*>(rhs.release()));
        get_deleter() = std::move(rhs.get_deleter());
        return *this;
    }

    void swap(unique_ptr& rhs) noexcept {
        using std::swap;
        swap(m_pair, rhs.m_pair);
    }

    ~unique_ptr() {
        reset();
    }

    T *release() noexcept {
        return std::exchange(scratch::get<0>(m_pair), nullptr);
    }
    void reset(T *p = nullptr) noexcept {
        T *old_p = std::exchange(scratch::get<0>(m_pair), p);
        if (old_p != nullptr) {
            get_deleter()(old_p);
        }
    }

    T *get() const noexcept { return scratch::get<0>(m_pair); }
    Deleter& get_deleter() noexcept { return scratch::get<1>(m_pair); }
    const Deleter& get_deleter() const noexcept { return scratch::get<1>(m_pair); }
    operator bool() const noexcept { return get() != nullptr; }

    T& operator*() const noexcept { return *get(); }
    T* operator->() const noexcept { return get(); }
};

template<class T, class D> bool operator==(const unique_ptr<T,D>& a, decltype(nullptr)) noexcept { return a.get() == nullptr; }
template<class T, class D> bool operator!=(const unique_ptr<T,D>& a, decltype(nullptr)) noexcept { return a.get() != nullptr; }
template<class T, class D> bool operator==(decltype(nullptr), const unique_ptr<T,D>& a) noexcept { return a.get() == nullptr; }
template<class T, class D> bool operator!=(decltype(nullptr), const unique_ptr<T,D>& a) noexcept { return a.get() != nullptr; }

template<class T, class D, class U, class E> bool operator==(const unique_ptr<T,D>& a, const unique_ptr<U,E>& b) noexcept { return a.get() == b.get(); }
template<class T, class D, class U, class E> bool operator!=(const unique_ptr<T,D>& a, const unique_ptr<U,E>& b) noexcept { return a.get() != b.get(); }

template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace scratch
