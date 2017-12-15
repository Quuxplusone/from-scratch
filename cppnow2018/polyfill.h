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

template<class T>
constexpr bool is_nothrow_swappable_f() {
    T *p;
    using std::swap;
    return noexcept(swap(*p, *p));
}

template<class... Ts>
using is_nothrow_swappable = std::bool_constant<is_nothrow_swappable_f<Ts...>()>;

#define VV(name) template<class... Ts> constexpr bool name##_v = name<Ts...>::value
VV(is_assignable);
VV(is_constructible);
VV(is_convertible);
VV(is_copy_assignable);
VV(is_copy_constructible);
VV(is_destructible);
VV(is_move_assignable);
VV(is_move_constructible);
VV(is_nothrow_assignable);
VV(is_nothrow_constructible);
VV(is_nothrow_copy_assignable);
VV(is_nothrow_copy_constructible);
VV(is_nothrow_move_assignable);
VV(is_nothrow_move_constructible);
VV(is_nothrow_swappable);
VV(is_object);
VV(is_reference);
VV(is_same);
VV(is_scalar);
VV(is_trivially_assignable);
VV(is_trivially_constructible);
VV(is_trivially_copyable);
VV(is_trivially_destructible);
#undef VV

} // namespace std

#endif
