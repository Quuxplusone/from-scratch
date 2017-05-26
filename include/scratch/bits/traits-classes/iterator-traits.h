#pragma once

#include "scratch/bits/algorithm/iterator-tags.h"

#include <cstddef>

namespace scratch::detail {

template<class D, class V, class P, class R, class C>
struct iterator_traits_impl {
    using difference_type = D;
    using value_type = V;
    using pointer = P;
    using reference = R;
    using iterator_category = C;
};

} // namespace scratch::detail

namespace scratch {

template<class It>
struct iterator_traits : detail::iterator_traits_impl<
    typename It::difference_type,
    typename It::value_type,
    typename It::pointer,
    typename It::reference,
    typename It::iterator_category
> {};

template<class T>
struct iterator_traits<T*> : detail::iterator_traits_impl<
    ptrdiff_t,
    T,
    T*,
    T&,
    random_access_iterator_tag
> {};

template<class T>
struct iterator_traits<const T*> : detail::iterator_traits_impl<
    ptrdiff_t,
    T,
    const T*,
    const T&,
    random_access_iterator_tag
> {};

// The following convenience aliases are NOT standard!
template<class It> using iterator_value_type_t = typename iterator_traits<It>::value_type;
template<class It> using iterator_difference_t = typename iterator_traits<It>::difference_type;
template<class It> using iterator_category_t = typename iterator_traits<It>::iterator_category;

} // namespace scratch
