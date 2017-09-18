#pragma once

#include <utility>

namespace scratch {

template<class T = void> struct less {
    constexpr bool operator()(const T& lhs, const T& rhs) const { return lhs < rhs; }
};
template<class T = void> struct less_equal {
    constexpr bool operator()(const T& lhs, const T& rhs) const { return lhs <= rhs; }
};
template<class T = void> struct greater {
    constexpr bool operator()(const T& lhs, const T& rhs) const { return lhs > rhs; }
};
template<class T = void> struct greater_equal {
    constexpr bool operator()(const T& lhs, const T& rhs) const { return lhs >= rhs; }
};
template<class T = void> struct equal_to {
    constexpr bool operator()(const T& lhs, const T& rhs) const { return lhs == rhs; }
};
template<class T = void> struct not_equal_to {
    constexpr bool operator()(const T& lhs, const T& rhs) const { return lhs != rhs; }
};

template<> struct less<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) < std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct less_equal<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) <= std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct greater<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) > std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct greater_equal<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) >= std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct equal_to<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) == std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct not_equal_to<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) != std::forward<U>(u); }
    using is_transparent = void;
};

} // namespace scratch
