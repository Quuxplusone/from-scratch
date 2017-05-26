#pragma once

#include "scratch/bits/algorithm/iterator-tags.h"
#include "scratch/bits/type-traits/void-t.h"

#include <cstddef>

namespace scratch::detail {

template<class It, class = void>
struct iterator_traits_impl {
    // empty
};

template<class It>
struct iterator_traits_impl<It, void_t<
    typename It::difference_type,
    typename It::value_type,
    typename It::pointer,
    typename It::reference,
    typename It::iterator_category
>> {
    using difference_type = typename It::difference_type;
    using value_type = typename It::value_type;
    using pointer = typename It::pointer;
    using reference = typename It::reference;
    using iterator_category = typename It::iterator_category;
};

} // namespace scratch::detail

namespace scratch {

template<class It> struct iterator_traits : detail::iterator_traits_impl<It> {};

template<class T>
struct iterator_traits<T*> {
    using difference_type = ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = random_access_iterator_tag;
};

template<class T>
struct iterator_traits<const T*> {
    using difference_type = ptrdiff_t;
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_category = random_access_iterator_tag;
};

// The following convenience aliases are NOT standard!
template<class It> using iterator_value_type_t = typename iterator_traits<It>::value_type;
template<class It> using iterator_difference_t = typename iterator_traits<It>::difference_type;
template<class It> using iterator_category_t = typename iterator_traits<It>::iterator_category;

} // namespace scratch
