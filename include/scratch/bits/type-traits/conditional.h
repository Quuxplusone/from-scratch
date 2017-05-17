#pragma once

namespace scratch {

template<bool Condition, typename T, typename F> struct conditional {};
template<typename T, typename F> struct conditional<true, T, F> { using type = T; };
template<typename T, typename F> struct conditional<false, T, F> { using type = F; };

template<bool Condition, typename T, typename F> using conditional_t = typename conditional<Condition, T, F>::type;

} // namespace scratch
