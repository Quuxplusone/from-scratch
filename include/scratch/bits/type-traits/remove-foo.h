#pragma once

namespace scratch {

template<class T> struct remove_const                      { using type = T; };
template<class T> struct remove_const<const T>             { using type = T; };

template<class T> struct remove_cv                         { using type = T; };
template<class T> struct remove_cv<const T>                { using type = T; };
template<class T> struct remove_cv<volatile T>             { using type = T; };
template<class T> struct remove_cv<const volatile T>       { using type = T; };

template<class T> struct remove_volatile                   { using type = T; };
template<class T> struct remove_volatile<volatile T>       { using type = T; };

template<class T> struct remove_pointer                    { using type = T; };
template<class T> struct remove_pointer<T*>                { using type = T; };
template<class T> struct remove_pointer<T* const>          { using type = T; };
template<class T> struct remove_pointer<T* volatile>       { using type = T; };
template<class T> struct remove_pointer<T* const volatile> { using type = T; };

template<class T> struct remove_reference                  { using type = T; };
template<class T> struct remove_reference<T&>              { using type = T; };
template<class T> struct remove_reference<T&&>             { using type = T; };

template<class T> using remove_const_t = typename remove_const<T>::type;
template<class T> using remove_cv_t = typename remove_cv<T>::type;
template<class T> using remove_pointer_t = typename remove_pointer<T>::type;
template<class T> using remove_reference_t = typename remove_reference<T>::type;
template<class T> using remove_volatile_t = typename remove_volatile<T>::type;

} // namespace scratch
