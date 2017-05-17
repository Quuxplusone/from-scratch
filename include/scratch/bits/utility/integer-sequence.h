#pragma once

#include <cstddef>

namespace scratch {

template<typename T, T... Is>
struct integer_sequence {
    using value_type = T;
    static constexpr size_t size() noexcept { return sizeof...(Is); }
};

template<size_t... Ints>
using index_sequence = integer_sequence<size_t, Ints...>;

namespace detail {

template<typename T, typename IndexSequence>
struct index_to_integer;

template<typename T, size_t... Is>
struct index_to_integer<T, index_sequence<Is...>> {
    using type = integer_sequence<T, size_t(Is)...>;
};

template<size_t Size> struct make_index_sequence_impl;
template<typename IndexSequence> struct index_sequence_extend;

template<size_t... Is>
struct index_sequence_extend<index_sequence<Is...>> {
    using type = index_sequence<Is..., sizeof...(Is)>;
};

template<size_t Size>
struct make_index_sequence_impl {
    static_assert(Size >= 0);
    using prior_type = typename make_index_sequence_impl<Size - 1>::type;
    using type = typename index_sequence_extend<prior_type>::type;
};

template<>
struct make_index_sequence_impl<0> {
    using type = index_sequence<>;
};

} // namespace detail

template<size_t Size>
using make_index_sequence = typename detail::make_index_sequence_impl<Size>::type;

template<typename... Ts>
using index_sequence_for = make_index_sequence<sizeof...(Ts)>;

template<typename T, T Size>
using make_integer_sequence = typename detail::index_to_integer<T, make_index_sequence<size_t(Size)>>::type;

} // namespace scratch
