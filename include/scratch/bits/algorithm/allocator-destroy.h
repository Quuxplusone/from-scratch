#pragma once

#include "scratch/bits/traits-classes/allocator-traits.h"

namespace scratch {

template<class T, class Alloc>
void destroy_at(T *p, Alloc& a)
{
    allocator_traits<Alloc>::destroy(a, p);
}

template<class FwdIt, class Alloc>
void destroy(FwdIt first, FwdIt last, Alloc& a)
{
    while (first != last) {
        scratch::destroy_at(&*first, a);
        ++first;
    }
}

template<class FwdIt, class Size, class Alloc>
void destroy_n(FwdIt first, Size count, Alloc& a)
{
    while (count != 0) {
        scratch::destroy_at(&*first, a);
        ++first;
        --count;
    }
}

} // namespace scratch
