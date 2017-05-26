#pragma once

#include "scratch/bits/algorithm/iterator-tags.h"
#include "scratch/bits/traits-classes/iterator-traits.h"

namespace scratch::detail {

template<class It, class Distance>
constexpr void advance_impl(It& it, Distance n, random_access_iterator_tag) {
    it += n;
}

template<class It, class Distance>
constexpr void advance_impl(It& it, Distance n, bidirectional_iterator_tag) {
    if (n >= 0) {
        while (n != 0) {
            ++it;
            --n;
        }
    } else {
        while (n != 0) {
            --it;
            ++n;
        }
    }
}

template<class It, class Distance>
constexpr void advance_impl(It& it, Distance n, input_iterator_tag) {
    while (n != 0) {
        ++it;
        --n;
    }
}

template<class It>
constexpr auto distance_impl(It first, It last, random_access_iterator_tag) {
    return last - first;
}

template<class It>
constexpr auto distance_impl(It first, It last, input_iterator_tag) {
    iterator_difference_t<It> n = 0;
    while (first != last) {
        ++first;
        ++n;
    }
    return n;
}

} // namespace scratch::detail

namespace scratch {

template<class It, class Distance>
constexpr void advance(It& it, Distance n)
{
    detail::advance_impl(it, n, iterator_category_t<It>{});
}

template<class It>
constexpr iterator_difference_t<It> distance(It first, It last)
{
    return detail::distance_impl(first, last, iterator_category_t<It>{});
}

template<class It>
constexpr It next(It it, iterator_difference_t<It> n = 1)
{
    detail::advance_impl(it, n, iterator_category_t<It>{});
    return it;
}

template<class It>
constexpr It prev(It it, iterator_difference_t<It> n = 1)
{
    detail::advance_impl(it, -n, iterator_category_t<It>{});
    return it;
}

} // namespace scratch
