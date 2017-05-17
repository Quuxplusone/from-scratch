#pragma once

namespace scratch {

template<bool Condition, typename T = void> struct enable_if {};
template<typename T> struct enable_if<true, T> { using type = T; };

template<bool Condition, typename T = void> using enable_if_t = typename enable_if<Condition, T>::type;

// bool_if_t is NOT a standard type trait.
template<bool Condition> using bool_if_t = enable_if_t<Condition, bool>;

} // namespace scratch
