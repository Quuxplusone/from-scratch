#pragma once

#include "scratch/bits/type-traits/compiler-magic.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/integral-constant.h"

#include <cstddef>
#include <utility>

namespace scratch {

template<typename T, size_t Index, typename = void>
class compressed_element
{
    T t;
public:
    compressed_element() = default;

    template<typename U = T>
    compressed_element(U&& u) : t(std::forward<U>(u)) {}

    T& get() { return t; }
    const T& get() const { return t; }

    T& get_element(index_constant<Index>) & { return t; }
    T&& get_element(index_constant<Index>) && { return std::move(t); }
    const T& get_element(index_constant<Index>) const & { return t; }
    const T&& get_element(index_constant<Index>) const && { return std::move(t); }
};

template<typename T, size_t Index>
class compressed_element<T, Index, enable_if_t<is_empty_v<T> && !is_final_v<T>>>
    : private T
{
public:
    compressed_element() = default;

    template<typename U = T>
    compressed_element(U&& u) : T(std::forward<U>(u)) {}

    T& get_element(index_constant<Index>) & { return *this; }
    T&& get_element(index_constant<Index>) && { return std::move(*this); }
    const T& get_element(index_constant<Index>) const & { return *this; }
    const T&& get_element(index_constant<Index>) const && { return std::move(*this); }
};


template<typename, typename> class compressed_pair;

template<size_t I, typename... Ts> constexpr inline decltype(auto) get(compressed_pair<Ts...>& t) { return t.get_element(index_constant<I>{}); }
template<size_t I, typename... Ts> constexpr inline decltype(auto) get(compressed_pair<Ts...>&& t) { return std::move(t).get_element(index_constant<I>{}); }
template<size_t I, typename... Ts> constexpr inline decltype(auto) get(const compressed_pair<Ts...>& t) { return t.get_element(index_constant<I>{}); }
template<size_t I, typename... Ts> constexpr inline decltype(auto) get(const compressed_pair<Ts...>&& t) { return std::move(t).get_element(index_constant<I>{}); }


template<typename T1, typename T2>
class compressed_pair :
    private compressed_element<T1, 0>,
    private compressed_element<T2, 1>
{
    using E1 = compressed_element<T1, 0>;
    using E2 = compressed_element<T2, 1>;

public:
    using E1::get_element;
    using E2::get_element;

    constexpr compressed_pair() = default;

    template<typename U1 = T1, typename U2 = T2>
    constexpr compressed_pair(U1&& u1, U2&& u2) : E1(std::forward<U1>(u1)), E2(std::forward<U2>(u2)) {}

    void swap(compressed_pair& rhs) {
        using std::swap;
        swap(scratch::get<0>(*this), scratch::get<0>(rhs));
        swap(scratch::get<1>(*this), scratch::get<1>(rhs));
    }
};

template<typename T1, typename T2>
void swap(compressed_pair<T1, T2>& a, compressed_pair<T1, T2>& b)
{
    a.swap(b);
}

} // namespace scratch
