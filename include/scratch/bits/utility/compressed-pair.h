#pragma once

#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/utility/compressed-element.h"

#include <cstddef>
#include <utility>

namespace scratch {

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
    static_assert(is_object_v<T1> && is_object_v<T2>, "Both type parameters to compressed_pair must be object types");

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
