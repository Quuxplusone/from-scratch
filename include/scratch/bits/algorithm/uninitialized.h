#pragma once

#include "scratch/bits/traits-classes/iterator-traits.h"
#include "scratch/bits/algorithm/destroy.h"
#include "scratch/bits/algorithm/move-iterator.h"

#include <new>

namespace scratch::detail {

template<class FwdIt, class F>
void uninitialized_construct_via(FwdIt first, FwdIt last, const F& construct)
{
    FwdIt current = first;
    try {
        while (current != last) {
            construct(&*current);  // Construct an object at "current".
            ++current;
        }
    } catch (...) {
        scratch::destroy(first, current);
        throw;
    }
}

template<class FwdIt, class Size, class F>
void uninitialized_construct_n_via(FwdIt first, Size count, const F& construct)
{
    FwdIt current = first;
    try {
        while (count > 0) {
            construct(&*current);  // Construct an object at "current".
            ++current;
            --count;
        }
    } catch (...) {
        scratch::destroy(first, current);
        throw;
    }
    return current;
}

} // namespace scratch::detail

namespace scratch {

template<class It, class FwdIt>
FwdIt uninitialized_copy(It first, It last, FwdIt dest)
{
    using T = iterator_value_type_t<FwdIt>;
    FwdIt current = dest;
    try {
        while (first != last) {
            ::new (&*current) T(*first);
            ++first;
            ++current;
        }
    } catch (...) {
        scratch::destroy(dest, current);
        throw;
    }
    return current;
}

template<class It, class FwdIt>
FwdIt uninitialized_move(It first, It last, FwdIt dest)
{
    return uninitialized_copy(
        scratch::move_iterator(first),
        scratch::move_iterator(last),
        dest
    ).base();
}

template<class It, class FwdIt>
FwdIt uninitialized_move_if_noexcept(It first, It last, FwdIt dest)
{
    if constexpr (is_constructible_v<iterator_value_type_t<FwdIt>, const iterator_value_type_t<It>&> &&
                  !is_nothrow_constructible_v<iterator_value_type_t<FwdIt>, iterator_value_type_t<It>&&>) {
        return uninitialized_copy(first, last, dest);
    } else {
        return uninitialized_move(first, last, dest);
    }
}

template<class It, class Size, class FwdIt>
FwdIt uninitialized_copy_n(It first, Size count, FwdIt dest)
{
    using T = iterator_value_type_t<FwdIt>;
    return detail::uninitialized_construct_n_via(dest, count, [&](T *p) {
        ::new (p) T(*first);
        ++first;
    });
}

template<class It, class Size, class FwdIt>
FwdIt uninitialized_move_n(It first, Size count, FwdIt dest)
{
    return uninitialized_copy_n(
        scratch::move_iterator(first),
        count,
        dest
    ).base();
}

template<class It, class Size, class FwdIt>
FwdIt uninitialized_move_if_noexcept_n(It first, Size count, FwdIt dest)
{
    if constexpr (is_constructible_v<iterator_value_type_t<FwdIt>, const iterator_value_type_t<It>&> &&
                  !is_nothrow_constructible_v<iterator_value_type_t<FwdIt>, iterator_value_type_t<It>&&>) {
        return uninitialized_copy_n(first, count, dest);
    } else {
        return uninitialized_move_n(first, count, dest);
    }
}

template<class FwdIt, class T>
void uninitialized_fill(FwdIt dest, FwdIt last, const T& value)
{
    using T = iterator_value_type_t<FwdIt>;
    detail::uninitialized_construct_via(dest, last, [&](T *p) {
        ::new (p) T(value);
    });
}

template<class FwdIt, class Size, class T>
FwdIt uninitialized_fill_n(FwdIt dest, Size count, const T& value)
{
    using T = iterator_value_type_t<FwdIt>;
    return detail::uninitialized_construct_n_via(dest, count, [&](T *p) {
        ::new (p) T(value);
    });
}

template<class FwdIt>
void uninitialized_default_construct(FwdIt dest, FwdIt last)
{
    using T = iterator_value_type_t<FwdIt>;
    detail::uninitialized_construct_via(dest, last, [](T *p) {
        ::new (p) T;
    });
}

template<class FwdIt, class Size>
FwdIt uninitialized_default_construct_n(FwdIt dest, Size count)
{
    using T = iterator_value_type_t<FwdIt>;
    return detail::uninitialized_construct_n_via(dest, count, [](T *p) {
        ::new (p) T;
    });
}

template<class FwdIt>
void uninitialized_value_construct(FwdIt dest, FwdIt last)
{
    using T = iterator_value_type_t<FwdIt>;
    detail::uninitialized_construct_via(dest, last, [](T *p) {
        ::new (p) T();
    });
}

template<class FwdIt, class Size>
void uninitialized_value_construct_n(FwdIt dest, Size count)
{
    using T = iterator_value_type_t<FwdIt>;
    return detail::uninitialized_construct_n_via(dest, count, [](T *p) {
        ::new (p) T();
    });
}

} // namespace scratch
