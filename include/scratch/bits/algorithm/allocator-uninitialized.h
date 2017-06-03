#pragma once

#include "scratch/bits/algorithm/allocator-destroy.h"
#include "scratch/bits/algorithm/move-iterator.h"
#include "scratch/bits/traits-classes/allocator-traits.h"
#include "scratch/bits/traits-classes/iterator-traits.h"
#include "scratch/bits/type-traits/is-fooible.h"

namespace scratch::detail {

template<class FwdIt, class C, class Alloc>
void uninitialized_construct_via(FwdIt first, FwdIt last, const C& construct, Alloc& a)
{
    FwdIt current = first;
    try {
        while (current != last) {
            construct(&*current);  // Construct an object at "current".
            ++current;
        }
    } catch (...) {
        scratch::destroy(first, current, a);
        throw;
    }
}

template<class FwdIt, class Size, class C, class Alloc>
void uninitialized_construct_n_via(FwdIt first, Size count, const C& construct, const Alloc& a)
{
    FwdIt current = first;
    try {
        while (count > 0) {
            construct(&*current);  // Construct an object at "current".
            ++current;
            --count;
        }
    } catch (...) {
        scratch::destroy(first, current, a);
        throw;
    }
    return current;
}

} // namespace scratch::detail

namespace scratch {

template<class It, class FwdIt, class Alloc>
FwdIt uninitialized_copy(It first, It last, FwdIt dest, Alloc& a)
{
    using T = iterator_value_type_t<FwdIt>;
    static_assert(is_same_v<T, typename allocator_traits<Alloc>::value_type>);
    FwdIt current = dest;
    try {
        while (first != last) {
            allocator_traits<Alloc>::construct(a, &*current, *first);
            ++first;
            ++current;
        }
    } catch (...) {
        scratch::destroy(dest, current, a);
        throw;
    }
    return current;
}

template<class It, class FwdIt, class Alloc>
FwdIt uninitialized_move(It first, It last, FwdIt dest, Alloc& a)
{
    return uninitialized_copy(
        scratch::move_iterator(first),
        scratch::move_iterator(last),
        dest,
        a
    );
}

template<class It, class FwdIt, class Alloc>
FwdIt uninitialized_move_if_noexcept(It first, It last, FwdIt dest, Alloc& a)
{
    if constexpr (is_constructible_v<iterator_value_type_t<FwdIt>, const iterator_value_type_t<It>&> &&
                  !is_nothrow_constructible_v<iterator_value_type_t<FwdIt>, iterator_value_type_t<It>&&>) {
        return uninitialized_copy(first, last, dest, a);
    } else {
        return uninitialized_move(first, last, dest, a);
    }
}

template<class It, class Size, class FwdIt, class Alloc>
FwdIt uninitialized_copy_n(It first, Size count, FwdIt dest, Alloc& a)
{
    using T = iterator_value_type_t<FwdIt>;
    static_assert(is_same_v<T, typename allocator_traits<Alloc>::value_type>);
    return detail::uninitialized_construct_n_via(dest, count, [&](T *p) {
        allocator_traits<Alloc>::construct(a, p, *first);
        ++first;
    }, a);
}

template<class It, class Size, class FwdIt, class Alloc>
FwdIt uninitialized_move_n(It first, Size count, FwdIt dest, Alloc& a)
{
    return uninitialized_copy_n(
        scratch::move_iterator(first),
        count,
        dest,
        a
    );
}

template<class It, class Size, class FwdIt, class Alloc>
FwdIt uninitialized_move_if_noexcept_n(It first, Size count, FwdIt dest, Alloc& a)
{
    if constexpr (is_constructible_v<iterator_value_type_t<FwdIt>, const iterator_value_type_t<It>&> &&
                  !is_nothrow_constructible_v<iterator_value_type_t<FwdIt>, iterator_value_type_t<It>&&>) {
        return uninitialized_copy(first, count, dest, a);
    } else {
        return uninitialized_move(first, count, dest, a);
    }
}

template<class FwdIt, class U, class Alloc>
void uninitialized_fill(FwdIt dest, FwdIt last, const U& value, Alloc& a)
{
    using T = iterator_value_type_t<FwdIt>;
    static_assert(is_same_v<T, typename allocator_traits<Alloc>::value_type>);
    detail::uninitialized_construct_via(dest, last, [&](T *p) {
        allocator_traits<Alloc>::construct(a, p, value);
    }, a);
}

template<class FwdIt, class Size, class U, class Alloc>
FwdIt uninitialized_fill_n(FwdIt dest, Size count, const U& value, Alloc& a)
{
    using T = iterator_value_type_t<FwdIt>;
    static_assert(is_same_v<T, typename allocator_traits<Alloc>::value_type>);
    return detail::uninitialized_construct_n_via(dest, count, [&](T *p) {
        allocator_traits<Alloc>::construct(a, p, value);
    }, a);
}

template<class FwdIt, class Alloc>
void uninitialized_default_construct(FwdIt dest, FwdIt last, Alloc& a)
{
    using T = iterator_value_type_t<FwdIt>;
    static_assert(is_same_v<T, typename allocator_traits<Alloc>::value_type>);
    detail::uninitialized_construct_via(dest, last, [&](T *p) {
        allocator_traits<Alloc>::default_construct(a, p);
    }, a);
}

template<class FwdIt, class Size, class Alloc>
FwdIt uninitialized_default_construct_n(FwdIt dest, Size count, Alloc& a)
{
    using T = iterator_value_type_t<FwdIt>;
    static_assert(is_same_v<T, typename allocator_traits<Alloc>::value_type>);
    return detail::uninitialized_construct_n_via(dest, count, [&](T *p) {
        allocator_traits<Alloc>::default_construct(a, p);
    }, a);
}

template<class FwdIt, class Alloc>
void uninitialized_value_construct(FwdIt dest, FwdIt last, Alloc& a)
{
    using T = iterator_value_type_t<FwdIt>;
    static_assert(is_same_v<T, typename allocator_traits<Alloc>::value_type>);
    detail::uninitialized_construct_via(dest, last, [&](T *p) {
        allocator_traits<Alloc>::construct(a, p);
    }, a);
}

template<class FwdIt, class Size, class Alloc>
void uninitialized_value_construct_n(FwdIt dest, Size count, Alloc& a)
{
    using T = iterator_value_type_t<FwdIt>;
    static_assert(is_same_v<T, typename allocator_traits<Alloc>::value_type>);
    return detail::uninitialized_construct_n_via(dest, count, [&](T *p) {
        allocator_traits<Alloc>::construct(a, p);
    }, a);
}

} // namespace scratch
