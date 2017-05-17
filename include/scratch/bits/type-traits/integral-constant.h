#pragma once

#include <cstddef>

namespace scratch {

template<typename Type, Type V>
struct integral_constant {
    using type = integral_constant;
    using value_type = Type;

    static constexpr Type value = V;

    constexpr operator value_type() const noexcept { return V; }
    constexpr value_type operator()() const noexcept { return V; }
};

template <bool B> using bool_constant = integral_constant<bool, B>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

// index_constant is NOT a standard library type.
template <size_t I> using index_constant = integral_constant<size_t, I>;

} // namespace scratch
