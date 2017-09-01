#pragma once

#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/utility/declval.h"

namespace scratch::detail {

template<class G, class T, class Enable> struct is_seed_generator_impl                                                            : false_type {};
template<class G, class T>               struct is_seed_generator_impl<G, T, decltype(void(declval<G>().generate((T*)0, (T*)0)))> : true_type {};

} // namespace scratch::detail

namespace scratch {

template<class G, class T = unsigned> struct is_seed_generator : detail::is_seed_generator_impl<G, T, void> {};

template<class G, class T = unsigned> inline constexpr bool is_seed_generator_v = is_seed_generator<G, T>::value;

} // namespace std
