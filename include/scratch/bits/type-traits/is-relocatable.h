#pragma once

#include "scratch/bits/type-traits/is-fooible.h"

namespace scratch {

template<class T> inline constexpr bool is_relocatable_v = is_move_constructible_v<T> && is_destructible_v<T>;

template<class T> struct is_relocatable : bool_constant<is_relocatable_v<T>> {};

// The user may want to customize this trait.
template<class T> struct is_trivially_relocatable :
    bool_constant<is_trivially_move_constructible_v<T> && is_trivially_destructible_v<T>> {};

template<class T> inline constexpr bool is_trivially_relocatable_v = is_trivially_relocatable<T>::value;

} // namespace scratch
