#pragma once

#include "scratch/bits/type-traits/integral-constant.h"

namespace scratch {

template<class T> struct is_noncv_floating_point                  : false_type {};
template<>        struct is_noncv_floating_point<float>           : true_type {};
template<>        struct is_noncv_floating_point<double>          : true_type {};
template<>        struct is_noncv_floating_point<long double>     : true_type {};

template <class T> struct is_noncv_integral                       : false_type {};
template <>        struct is_noncv_integral<bool>                 : true_type {};
template <>        struct is_noncv_integral<char>                 : true_type {};
template <>        struct is_noncv_integral<signed char>          : true_type {};
template <>        struct is_noncv_integral<unsigned char>        : true_type {};
template <>        struct is_noncv_integral<wchar_t>              : true_type {};
template <>        struct is_noncv_integral<char16_t>             : true_type {};
template <>        struct is_noncv_integral<char32_t>             : true_type {};
template <>        struct is_noncv_integral<short>                : true_type {};
template <>        struct is_noncv_integral<unsigned short>       : true_type {};
template <>        struct is_noncv_integral<int>                  : true_type {};
template <>        struct is_noncv_integral<unsigned int>         : true_type {};
template <>        struct is_noncv_integral<long>                 : true_type {};
template <>        struct is_noncv_integral<unsigned long>        : true_type {};
template <>        struct is_noncv_integral<long long>            : true_type {};
template <>        struct is_noncv_integral<unsigned long long>   : true_type {};


template<class T> struct is_noncv_member_pointer                  : false_type {};
template<class T, class U> struct is_noncv_member_pointer<T U::*> : true_type {};

template<class T> struct is_noncv_null_pointer                    : false_type {};
template<>        struct is_noncv_null_pointer<decltype(nullptr)> : true_type {};

template<class T> struct is_noncv_pointer                         : false_type {};
template<class T> struct is_noncv_pointer<T*>                     : true_type {};

template<class T> struct is_noncv_void                            : false_type {};
template<>        struct is_noncv_void<void>                      : true_type {};

template<typename T> inline constexpr bool is_noncv_floating_point_v = is_noncv_floating_point<T>::value;
template<typename T> inline constexpr bool is_noncv_integral_v = is_noncv_integral<T>::value;
template<typename T> inline constexpr bool is_noncv_member_pointer_v = is_noncv_member_pointer<T>::value;
template<typename T> inline constexpr bool is_noncv_null_pointer_v = is_noncv_null_pointer<T>::value;
template<typename T> inline constexpr bool is_noncv_pointer_v = is_noncv_pointer<T>::value;
template<typename T> inline constexpr bool is_noncv_void_v = is_noncv_void<T>::value;

} // namespace scratch
