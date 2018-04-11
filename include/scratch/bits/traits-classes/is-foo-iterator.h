#pragma once

#include "scratch/bits/algorithm/iterator-tags.h"
#include "scratch/bits/traits-classes/iterator-traits.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-convertible.h"
#include "scratch/bits/type-traits/is-foo.h"

namespace scratch::detail {

template<class It, class Tag, class> struct is_iterator_impl                                                                       : false_type {};
template<class It, class Tag>        struct is_iterator_impl<It, Tag, enable_if_t<is_convertible_v<iterator_category_t<It>, Tag>>> : true_type {};

} // namespace scratch::detail

namespace scratch {

template<class It> struct is_input_iterator : detail::is_iterator_impl<It, input_iterator_tag, void> {};
template<class It> struct is_output_iterator : detail::is_iterator_impl<It, output_iterator_tag, void> {};
template<class It> struct is_forward_iterator : detail::is_iterator_impl<It, forward_iterator_tag, void> {};
template<class It> struct is_bidirectional_iterator : detail::is_iterator_impl<It, bidirectional_iterator_tag, void> {};
template<class It> struct is_random_access_iterator : detail::is_iterator_impl<It, random_access_iterator_tag, void> {};

template<class It> inline constexpr bool is_input_iterator_v = is_input_iterator<It>::value;
template<class It> inline constexpr bool is_output_iterator_v = is_output_iterator<It>::value;
template<class It> inline constexpr bool is_forward_iterator_v = is_forward_iterator<It>::value;
template<class It> inline constexpr bool is_bidirectional_iterator_v = is_bidirectional_iterator<It>::value;
template<class It> inline constexpr bool is_random_access_iterator_v = is_random_access_iterator<It>::value;

template<class It> inline constexpr bool is_iterator_v = is_input_iterator_v<It> || is_output_iterator_v<It>;
template<class It> struct is_iterator : bool_constant<is_iterator_v<It>> {};

template<class It> struct is_contiguous_iterator : is_pointer<It> {};
template<class It> inline constexpr bool is_contiguous_iterator_v = is_contiguous_iterator<It>::value;

} // namespace scratch
