#pragma once

#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-function.h"
#include "scratch/bits/utility/declval.h"

namespace scratch::detail {

template<typename From, typename To, bool = is_void_v<From> || is_function_v<To> || is_array_v<To>>
struct is_convertible_impl
{
    using type = bool_constant<is_void_v<To>>;
};

template<typename From, typename To>
class is_convertible_impl<From, To, false>
{
    template<typename T> static void convertible(T);

    template<typename F, typename = decltype(convertible<To>(declval<F>()))>
    static true_type test(int);

    template<typename>
    static false_type test(...);

public:
    using type = decltype(test<From>(0));
};

} // namespace scratch::detail

namespace scratch {

template<typename From, typename To>
struct is_convertible : public detail::is_convertible_impl<From, To>::type {};

template<typename From, typename To>
inline constexpr bool is_convertible_v = is_convertible<From, To>::value;

} // namespace scratch
