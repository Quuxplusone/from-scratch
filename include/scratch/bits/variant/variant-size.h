#pragma once

#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/utility/nth-t.h"

#include <cstddef>

namespace scratch {

template<typename...> class variant;

template<typename V> struct variant_size;
template<typename V> struct variant_size<const V> : variant_size<V> {};
template<typename V> struct variant_size<volatile V> : variant_size<V> {};
template<typename... Ts> struct variant_size<variant<Ts...>> : index_constant<sizeof...(Ts)> {};

template<typename V> inline constexpr size_t variant_size_v = variant_size<V>::value;

template<size_t I, typename V> struct variant_alternative;
template<size_t I, typename V> struct variant_alternative<I, const V> : variant_alternative<I, V> {};
template<size_t I, typename V> struct variant_alternative<I, volatile V> : variant_alternative<I, V> {};
template<size_t I, typename... Ts> struct variant_alternative<I, variant<Ts...>> {
    static_assert(I < sizeof...(Ts));
    using type = scratch::nth_t<I, Ts...>;
};

template<size_t I, typename V> using variant_alternative_t = typename variant_alternative<I, V>::type;

} // namespace scratch
