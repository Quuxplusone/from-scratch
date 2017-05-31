#pragma once

#include "scratch/bits/type-traits/priority-tag.h"
#include "scratch/bits/utility/declval.h"

#include <cstddef>

namespace scratch::detail {

template<class Ptr>
auto pointer_element_type(Ptr&, priority_tag<1>) -> typename Ptr::element_type;

template<template<class...> class SomePointer, class T, class... Vs>
auto pointer_element_type(SomePointer<T, Vs...>&, priority_tag<0>) -> T;

template<class P> auto pointer_difference_type(priority_tag<1>) -> typename P::difference_type;
template<class P> auto pointer_difference_type(priority_tag<0>) -> ptrdiff_t;
template<class P> using pointer_difference_type_t = decltype(pointer_difference_type<P>(priority_tag<1>{}));

template<class U, class Ptr>
auto pointer_rebind(Ptr&, priority_tag<1>) -> typename Ptr::template rebind<U>;

template<class U, template<class...> class SomePointer, class T, class... Vs>
auto pointer_rebind(SomePointer<T, Vs...>&, priority_tag<0>) -> SomePointer<U, Vs...>;

} // namespace scratch::detail

namespace scratch {

template<class Ptr>
struct pointer_traits
{
    using pointer = Ptr;
    using element_type =
        decltype(detail::pointer_element_type(declval<Ptr&>(), priority_tag<1>{}));
    using difference_type = detail::pointer_difference_type_t<Ptr>;

    template<class U> using rebind =
        decltype(detail::pointer_rebind<U>(declval<Ptr&>(), priority_tag<1>{}));
};

template<class T>
struct pointer_traits<T*>
{
    using pointer = T*;
    using element_type = T;
    using difference_type = ptrdiff_t;

    template<class U> using rebind = U*;
};

} // namespace scratch
