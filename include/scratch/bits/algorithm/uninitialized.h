#pragma once

#include "scratch/bits/algorithm/allocator-uninitialized.h"
#include "scratch/bits/containers/allocator.h"
#include "scratch/bits/traits-classes/iterator-traits.h"

namespace scratch {

template<class It, class FwdIt>
FwdIt uninitialized_copy(It first, It last, FwdIt dest)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    return uninitialized_copy(first, last, dest, a);
}

template<class It, class FwdIt>
FwdIt uninitialized_move(It first, It last, FwdIt dest)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    return uninitialized_move(first, last, dest, a);
}

template<class It, class FwdIt>
FwdIt uninitialized_move_if_noexcept(It first, It last, FwdIt dest)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    return uninitialized_move_if_noexcept(first, last, dest, a);
}

template<class It, class Size, class FwdIt>
FwdIt uninitialized_copy_n(It first, Size count, FwdIt dest)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    return uninitialized_copy_n(first, count, dest, a);
}

template<class It, class Size, class FwdIt>
FwdIt uninitialized_move_n(It first, Size count, FwdIt dest)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    return uninitialized_move_n(first, count, dest, a);
}

template<class It, class Size, class FwdIt>
FwdIt uninitialized_move_if_noexcept_n(It first, Size count, FwdIt dest)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    return uninitialized_move_if_noexcept_n(first, count, dest, a);
}

template<class FwdIt, class U>
void uninitialized_fill(FwdIt dest, FwdIt last, const U& value)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    return uninitialized_fill(dest, last, value, a);
}

template<class FwdIt, class Size, class U>
FwdIt uninitialized_fill_n(FwdIt dest, Size count, const U& value)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    return uninitialized_fill_n(dest, count, value, a);
}

template<class FwdIt>
void uninitialized_default_construct(FwdIt dest, FwdIt last)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    uninitialized_default_construct(dest, last, a);
}

template<class FwdIt, class Size>
FwdIt uninitialized_default_construct_n(FwdIt dest, Size count)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    uninitialized_default_construct_n(dest, count, a);
}

template<class FwdIt>
void uninitialized_value_construct(FwdIt dest, FwdIt last)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    uninitialized_value_construct(dest, last, a);
}

template<class FwdIt, class Size>
void uninitialized_value_construct_n(FwdIt dest, Size count)
{
    using T = iterator_value_type_t<FwdIt>;
    scratch::allocator<T> a;
    uninitialized_value_construct_n(dest, count, a);
}

} // namespace scratch
