#pragma once

#include "scratch/bits/type-traits/remove-foo.h"

namespace scratch::detail {

template<class T, class Enable> struct add_lvalue_reference_impl                            { using type = T; };
template<class T>               struct add_lvalue_reference_impl<T, remove_reference_t<T&>> { using type = T&; };

template<class T, class Enable> struct add_pointer_impl                                     { using type = T; };
template<class T>               struct add_pointer_impl<T, remove_pointer_t<T*>>            { using type = T*; };

template<class T, class Enable> struct add_rvalue_reference_impl                            { using type = T; };
template<class T>               struct add_rvalue_reference_impl<T, remove_reference_t<T&>> { using type = T&&; };

} // namespace scratch::detail

namespace scratch {

template<class T> struct add_const            { using type = const T; };
template<class T> struct add_cv               { using type = const volatile T; };
template<class T> struct add_lvalue_reference : detail::add_lvalue_reference_impl<T, remove_reference_t<T>> {};
template<class T> struct add_pointer          : detail::add_pointer_impl<remove_reference_t<T>, remove_reference_t<T>> {};
template<class T> struct add_rvalue_reference : detail::add_rvalue_reference_impl<T, remove_reference_t<T>> {};
template<class T> struct add_volatile         { using type = volatile T; };

template<class T> using add_const_t = typename add_const<T>::type;
template<class T> using add_cv_t = typename add_cv<T>::type;
template<class T> using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;
template<class T> using add_pointer_t = typename add_pointer<T>::type;
template<class T> using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;
template<class T> using add_volatile_t = typename add_volatile<T>::type;

} // namespace scratch
