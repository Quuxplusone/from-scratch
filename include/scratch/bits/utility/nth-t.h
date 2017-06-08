#pragma once

#include <cstddef>

namespace scratch {

template<size_t Index, typename T, typename... Rest> struct nth : nth<Index-1, Rest...> {};
template<typename T, typename... Rest> struct nth<0, T, Rest...> { using type = T; };

template<size_t Index, typename... Ts> using nth_t = typename nth<Index, Ts...>::type;

} // namespace scratch
