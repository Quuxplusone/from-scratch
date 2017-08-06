#pragma once

#include "scratch/bits/algorithm/move-iterator.h"
#include "scratch/bits/traits-classes/iterator-traits.h"
#include "scratch/bits/type-traits/is-fooible.h"

namespace scratch {

template<class It, class OutIt, class Pred>
OutIt copy_if(It first, It last, OutIt dest, Pred pred)
{
    while (first != last) {
        if (pred(*first)) {
            *dest = *first;
            ++dest;
        }
        ++first;
    }
    return dest;
}

template<class It, class OutIt>
OutIt copy(It first, It last, OutIt dest)
{
    auto always_true = [](auto&&){ return true; };
    return scratch::copy_if(first, last, dest, always_true);
}

template<class It, class OutIt, class Pred>
OutIt move_if(It first, It last, OutIt dest, Pred pred)
{
    return scratch::copy_if(move_iterator(first), move_iterator(last), dest, pred);
}

template<class It, class OutIt>
OutIt move(It first, It last, OutIt dest)
{
    return scratch::copy(scratch::move_iterator(first), scratch::move_iterator(last), dest);
}

template<class It, class OutIt>
OutIt move_if_noexcept(It first, It last, OutIt dest)
{
    if constexpr (is_constructible_v<iterator_value_type_t<OutIt>, const iterator_value_type_t<It>&> &&
                  !is_nothrow_constructible_v<iterator_value_type_t<OutIt>, iterator_value_type_t<It>&&>) {
        return scratch::copy(first, last, dest);
    } else {
        return scratch::move(first, last, dest);
    }
}

template<class It, class Size, class OutIt>
OutIt copy_n(It first, Size count, OutIt dest)
{
    while (count) {
        *dest = *first;
        ++dest;
        ++first;
        --count;
    }
    return dest;
}

template<class It, class Size, class OutIt>
OutIt move_n(It first, Size count, OutIt dest)
{
    return scratch::copy_n(scratch::move_iterator(first), count, dest);
}

template<class It, class Size, class FwdIt>
FwdIt move_if_noexcept_n(It first, Size count, FwdIt dest)
{
    if constexpr (is_constructible_v<iterator_value_type_t<FwdIt>, const iterator_value_type_t<It>&> &&
                  !is_nothrow_constructible_v<iterator_value_type_t<FwdIt>, iterator_value_type_t<It>&&>) {
        return scratch::copy(first, count, dest);
    } else {
        return scratch::move(first, count, dest);
    }
}

template<class FwdIt, class U>
void fill(FwdIt dest, FwdIt last, const U& value)
{
    while (dest != last) {
        *dest = value;
        ++dest;
        ++last;
    }
}

template<class OutIt, class Size, class U>
OutIt fill_n(OutIt dest, Size count, const U& value)
{
    while (count) {
        *dest = value;
        ++dest;
        --count;
    }
    return dest;
}

} // namespace scratch
