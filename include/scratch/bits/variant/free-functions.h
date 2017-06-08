#pragma once

#include "scratch/bits/stdexcept/bad-variant-access.h"
#include "scratch/bits/type-traits/integral-constant.h"

#include <cstddef>
#include <utility>

namespace scratch {

template<typename...> class variant;

template<size_t I, typename... Ts> constexpr inline decltype(auto) get(variant<Ts...>& v) {
    static_assert(I < sizeof...(Ts));
    if (v.index() != I) throw bad_variant_access();
    return v.get_element(index_constant<I>{});
}
template<size_t I, typename... Ts> constexpr inline decltype(auto) get(variant<Ts...>&& v) {
    static_assert(I < sizeof...(Ts));
    if (v.index() != I) throw bad_variant_access();
    return std::move(v.get_element(index_constant<I>{}));
}
template<size_t I, typename... Ts> constexpr inline decltype(auto) get(const variant<Ts...>& v) {
    static_assert(I < sizeof...(Ts));
    if (v.index() != I) throw bad_variant_access();
    return v.get_element(index_constant<I>{});
}
template<size_t I, typename... Ts> constexpr inline decltype(auto) get(const variant<Ts...>&& v) {
    static_assert(I < sizeof...(Ts));
    if (v.index() != I) throw bad_variant_access();
    return std::move(v.get_element(index_constant<I>{}));
}

template<size_t I, typename... Ts> constexpr inline bool holds_alternative(const variant<Ts...>& v) {
    static_assert(I < sizeof...(Ts));
    return (v.index() == I);
}

template<size_t I, typename... Ts> constexpr inline auto *get_if(variant<Ts...> *v) {
    static_assert(I < sizeof...(Ts));
    return (v != nullptr && v->index() == I) ? &v->get_element(index_constant<I>{}) : nullptr;
}
template<size_t I, typename... Ts> constexpr inline auto *get_if(const variant<Ts...> *v) {
    static_assert(I < sizeof...(Ts));
    return (v != nullptr && v->index() == I) ? &v->get_element(index_constant<I>{}) : nullptr;
}

} // namespace scratch
