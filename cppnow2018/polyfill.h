#pragma once

#if __cplusplus < 201703

#include <iterator>
#include <new>
#include <utility>

namespace std {

template<class ForwardIt>
void uninitialized_value_construct(ForwardIt first, ForwardIt last)
{
    typedef typename std::iterator_traits<ForwardIt>::value_type Value;
    ForwardIt current = first;
    try {
        for (; current != last; ++current) {
            ::new (static_cast<void*>(std::addressof(*current))) Value();
        }
    }  catch (...) {
        for (; first != current; ++first) {
            first->~Value();
        }
        throw;
    }
}

template<class T>
void destroy_at(T* p)
{
    p->~T();
}

template< class ForwardIt >
void destroy( ForwardIt first, ForwardIt last )
{
  for (; first != last; ++first)
    std::destroy_at(std::addressof(*first));
}

template<class InputIt, class ForwardIt>
ForwardIt uninitialized_move(InputIt first, InputIt last, ForwardIt d_first)
{
    typedef typename std::iterator_traits<ForwardIt>::value_type Value;
    ForwardIt current = d_first;
    try {
        for (; first != last; ++first, (void) ++current) {
            ::new (static_cast<void*>(std::addressof(*current))) Value(std::move(*first));
        }
        return current;
    } catch (...) {
        for (; d_first != current; ++d_first) {
            d_first->~Value();
        }
        throw;
    }
}

} // namespace std

#endif
