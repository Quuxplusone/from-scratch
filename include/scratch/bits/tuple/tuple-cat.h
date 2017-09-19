#pragma once

#include "scratch/bits/tuple/forward-as-tuple.h"
#include "scratch/bits/tuple/reftuple.h"
#include "scratch/bits/tuple/tuple.h"
#include "scratch/bits/tuple/tuple-size.h"
#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/utility/integer-sequence.h"

#include <cstddef>
#include <utility>

namespace scratch::detail {

template<size_t... Sizes>
struct tuple_cat_impl
{
    static constexpr size_t index_of_tuple(size_t Combined)
    {
        size_t arr[] = { Sizes... };
        size_t i = 0;
        while (Combined >= arr[i]) {
            Combined -= arr[i];
            ++i;
        }
        return i;
    }

    static constexpr size_t index_into_tuple(size_t Combined)
    {
        size_t arr[] = { Sizes... };
        size_t i = 0;
        while (Combined >= arr[i]) {
            Combined -= arr[i];
            ++i;
        }
        return Combined;
    }

    template<class ResultType, class T, size_t... Is>
    static auto result_impl(T reftuple_of_tuples, index_sequence<Is...>) {
        return ResultType(
            get<index_into_tuple(Is)>(
                get<index_of_tuple(Is)>(
                    reftuple_of_tuples
                )
            )...
        );
    }
public:
    template<class ResultType, class... Tuples>
    static auto result(Tuples&&... tuples) {
        using Combineds = make_index_sequence<(0 + ... + Sizes)>;
        auto reftuple_of_tuples = forward_as_tuple(std::forward<Tuples>(tuples)...);
        return result_impl<ResultType>(reftuple_of_tuples, Combineds{});
    }
};

static_assert(tuple_cat_impl<1,3,2>::index_of_tuple(0) == 0);
static_assert(tuple_cat_impl<1,3,2>::index_of_tuple(1) == 1);
static_assert(tuple_cat_impl<1,3,2>::index_of_tuple(2) == 1);
static_assert(tuple_cat_impl<1,3,2>::index_of_tuple(3) == 1);
static_assert(tuple_cat_impl<1,3,2>::index_of_tuple(4) == 2);
static_assert(tuple_cat_impl<1,3,2>::index_of_tuple(5) == 2);

static_assert(tuple_cat_impl<1,3,2>::index_into_tuple(0) == 0);
static_assert(tuple_cat_impl<1,3,2>::index_into_tuple(1) == 0);
static_assert(tuple_cat_impl<1,3,2>::index_into_tuple(2) == 1);
static_assert(tuple_cat_impl<1,3,2>::index_into_tuple(3) == 2);
static_assert(tuple_cat_impl<1,3,2>::index_into_tuple(4) == 0);
static_assert(tuple_cat_impl<1,3,2>::index_into_tuple(5) == 1);

} // namespace scratch::detail

namespace scratch {

template<class...> struct tuple_cat_result {};

template<> struct tuple_cat_result<> { using type = tuple<>; };

template<template<class...> class Tuple, class... As>
struct tuple_cat_result<Tuple<As...>> { using type = Tuple<As...>; };

template<template<class...> class Tuple, class... As, class... Bs, class... Rest>
struct tuple_cat_result<Tuple<As...>, Tuple<Bs...>, Rest...> : tuple_cat_result<Tuple<As..., Bs...>, Rest...> {};

template<class... Ts>
using tuple_cat_result_t = typename tuple_cat_result<Ts...>::type;

template<class... Tuples, class ResultType = tuple_cat_result_t<decay_t<Tuples>...>>
ResultType tuple_cat(Tuples&&... tuples)
{
    using Impl = detail::tuple_cat_impl< tuple_size_v<decay_t<Tuples>>... >;
    return Impl::template result<ResultType>(std::forward<Tuples>(tuples)...);
}

template<class... Tuples, class ResultType = tuple_cat_result_t<reftuple<>, decay_t<Tuples>...>>
ResultType reftuple_cat(Tuples&&... tuples)
{
    using Impl = detail::tuple_cat_impl< tuple_size_v<decay_t<Tuples>>... >;
    return Impl::template result<ResultType>(std::forward<Tuples>(tuples)...);
}

} // namespace scratch
