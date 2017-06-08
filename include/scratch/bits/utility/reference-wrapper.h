#pragma once

#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/utility/invoke.h"

#include <utility>

namespace scratch {

template <typename T>
class reference_wrapper
{
    T *m_ptr;

public:
    using type = T;

    reference_wrapper(T& t) noexcept : m_ptr(&t) {}
    reference_wrapper(T&&) = delete;

    operator T& () const noexcept { return *m_ptr; }
    T& get() const noexcept { return *m_ptr; }

    template<typename... Args>
    auto operator() (Args&&... args) const
        noexcept(noexcept( scratch::invoke(declval<T&>(), std::forward<Args>(args)...) ))
        -> decltype( scratch::invoke(declval<T&>(), std::forward<Args>(args)...) )
    {
        return scratch::invoke(get(), std::forward<Args>(args)...);
    }
};

template<class T> inline constexpr reference_wrapper<T> ref(T& t) { return reference_wrapper<T>(t); }
template<class T> inline constexpr reference_wrapper<T> ref(T&& t) = delete;
template<class T> inline constexpr reference_wrapper<T> cref(const T& t) { return reference_wrapper<const T>(t); }
template<class T> inline constexpr reference_wrapper<T> cref(T&& t) = delete;

} // namespace scratch
