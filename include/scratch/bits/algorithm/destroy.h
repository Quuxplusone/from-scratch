#pragma once

namespace scratch {

template<class T>
void destroy_at(T *p)
{
    p->~T();
}

template<class FwdIt>
void destroy(FwdIt first, FwdIt last)
{
    while (first != last) {
        scratch::destroy_at(&*first);
        ++first;
    }
}

template<class FwdIt, class Size>
void destroy_n(FwdIt first, Size count)
{
    while (count != 0) {
        scratch::destroy_at(&*first);
        ++first;
        --count;
    }
}

} // namespace scratch
