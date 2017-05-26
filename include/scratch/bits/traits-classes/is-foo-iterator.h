#pragma once

#include "scratch/bits/algorithm/iterator-tags.h"
#include "scratch/bits/traits-classes/iterator-traits.h"
#include "scratch/bits/type-traits/is-convertible.h"

namespace scratch {

template<class It> inline constexpr bool is_input_iterator_v = is_convertible_v<iterator_category_t<It>, input_iterator_tag>;
template<class It> inline constexpr bool is_output_iterator_v = is_convertible_v<iterator_category_t<It>, output_iterator_tag>;
template<class It> inline constexpr bool is_forward_iterator_v = is_convertible_v<iterator_category_t<It>, forward_iterator_tag>;
template<class It> inline constexpr bool is_bidirectional_iterator_v = is_convertible_v<iterator_category_t<It>, bidirectional_iterator_tag>;
template<class It> inline constexpr bool is_random_access_iterator_v = is_convertible_v<iterator_category_t<It>, random_access_iterator_tag>;

} // namespace scratch
