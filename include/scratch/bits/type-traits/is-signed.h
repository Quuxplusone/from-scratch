#pragma once

#include "scratch/bits/type-traits/compiler-magic.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"

namespace scratch::detail {

template<bool IsArithmetic, typename T> struct is_signed_impl                     : bool_constant<T(-1) < T(0)> {};
template<typename T>                    struct is_signed_impl<false, T>           : false_type {};

template<bool IsArithmetic, typename T> struct is_unsigned_impl                   : bool_constant<T(-1) < T(0)> {};
template<typename T>                    struct is_unsigned_impl<false, T>         : false_type {};

template<bool IsEnum, class T> struct make_signed_impl                            { using type = T; };
template<class T>              struct make_signed_impl<true, T>                   : make_signed_impl<false, underlying_type_t<T>> {};
template<>                     struct make_signed_impl<false, char>               { using type = signed char; };
template<>                     struct make_signed_impl<false, unsigned char>      { using type = signed char; };
template<>                     struct make_signed_impl<false, unsigned short>     { using type = signed short; };
template<>                     struct make_signed_impl<false, unsigned int>       { using type = signed int; };
template<>                     struct make_signed_impl<false, unsigned long>      { using type = signed long; };
template<>                     struct make_signed_impl<false, unsigned long long> { using type = signed long long; };

template<bool IsEnum, class T> struct make_unsigned_impl                          { using type = T; };
template<class T>              struct make_unsigned_impl<true, T>                 : make_unsigned_impl<false, underlying_type_t<T>> {};
template<>                     struct make_unsigned_impl<false, char>             { using type = unsigned char; };
template<>                     struct make_unsigned_impl<false, signed char>      { using type = unsigned char; };
template<>                     struct make_unsigned_impl<false, signed short>     { using type = unsigned short; };
template<>                     struct make_unsigned_impl<false, signed int>       { using type = unsigned int; };
template<>                     struct make_unsigned_impl<false, signed long>      { using type = unsigned long; };
template<>                     struct make_unsigned_impl<false, signed long long> { using type = unsigned long long; };

} // namespace scratch::detail

namespace scratch {

template<class T> struct is_signed     : detail::is_signed_impl<is_arithmetic_v<T>, T> {};
template<class T> struct is_unsigned   : detail::is_unsigned_impl<is_arithmetic_v<T>, T> {};
template<class T> struct make_signed   : detail::make_signed_impl<is_enum_v<T>, T> {};
template<class T> struct make_unsigned : detail::make_unsigned_impl<is_enum_v<T>, T> {};

template<class T> inline constexpr bool is_signed_v = is_signed<T>::value;
template<class T> inline constexpr bool is_unsigned_v = is_unsigned<T>::value;
template<class T> using make_signed_t = typename make_signed<T>::type;
template<class T> using make_unsigned_t = typename make_unsigned<T>::type;

} // namespace scratch
