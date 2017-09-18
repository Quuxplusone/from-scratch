#pragma once

#include <utility>

namespace scratch {

template<class T = void> struct plus {
    constexpr T operator()(const T& lhs, const T& rhs) const { return lhs + rhs; }
};
template<class T = void> struct minus {
    constexpr T operator()(const T& lhs, const T& rhs) const { return lhs - rhs; }
};
template<class T = void> struct multiplies {
    constexpr T operator()(const T& lhs, const T& rhs) const { return lhs * rhs; }
};
template<class T = void> struct divides {
    constexpr T operator()(const T& lhs, const T& rhs) const { return lhs / rhs; }
};
template<class T = void> struct modulus {
    constexpr T operator()(const T& lhs, const T& rhs) const { return lhs % rhs; }
};
template<class T = void> struct bit_and {
    constexpr T operator()(const T& lhs, const T& rhs) const { return lhs & rhs; }
};
template<class T = void> struct bit_or {
    constexpr T operator()(const T& lhs, const T& rhs) const { return lhs | rhs; }
};
template<class T = void> struct bit_xor {
    constexpr T operator()(const T& lhs, const T& rhs) const { return lhs ^ rhs; }
};
template<class T = void> struct logical_and {
    constexpr bool operator()(const T& lhs, const T& rhs) const { return lhs && rhs; }
};
template<class T = void> struct logical_or {
    constexpr bool operator()(const T& lhs, const T& rhs) const { return lhs || rhs; }
};
template<class T = void> struct negate {
    constexpr T operator()(const T& lhs) const { return -lhs; }
};
template<class T = void> struct bit_not {
    constexpr T operator()(const T& lhs) const { return ~lhs; }
};
template<class T = void> struct logical_not {
    constexpr bool operator()(const T& lhs) const { return !lhs; }
};

template<> struct plus<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) + std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct minus<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) - std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct multiplies<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) * std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct divides<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) / std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct modulus<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) % std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct bit_and<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) & std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct bit_or<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) | std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct bit_xor<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) ^ std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct logical_and<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) && std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct logical_or<void> {
    template<class T, class U>
    constexpr auto operator()(T&& t, U&& u) const { return std::forward<T>(t) || std::forward<U>(u); }
    using is_transparent = void;
};
template<> struct negate<void> {
    template<class T>
    constexpr auto operator()(T&& t) const { return -std::forward<T>(t); }
    using is_transparent = void;
};
template<> struct bit_not<void> {
    template<class T>
    constexpr auto operator()(T&& t) const { return ~std::forward<T>(t); }
    using is_transparent = void;
};
template<> struct logical_not<void> {
    template<class T>
    constexpr auto operator()(T&& t) const { return !std::forward<T>(t); }
    using is_transparent = void;
};

} // namespace scratch
