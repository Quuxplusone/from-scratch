#pragma once

#include "scratch/bits/stdexcept/bad-variant-access.h"
#include "scratch/bits/type-traits/remove-foo.h"
#include "scratch/bits/utility/integer-sequence.h"
#include "scratch/bits/utility/invoke.h"
#include "scratch/bits/variant/free-functions.h"
#include "scratch/bits/variant/variant-size.h"

#include <cstddef>
#include <utility>

namespace scratch::detail {

// This function takes a list of indices and maxima, and combines them into
// one big "combined index".
template<class... Size_ts, size_t... Maxes>
constexpr size_t vv_combine_indices(index_sequence<Maxes...>, Size_ts... indices)
{
    size_t result = 0;
    char computation[] = {
        [&]() { result *= Maxes; result += indices; return '\0'; }() ...
    };
    (void)computation;
    return result;
}

static_assert(vv_combine_indices(index_sequence<10,10,10>{}, 1, 2, 3) == 123);
static_assert(vv_combine_indices(index_sequence<2,3,4>{}, 1, 2, 2) == 22);


// This function takes a list of maxima, a single "combined index", and
// a "rank"; and pulls out just the original index corresponding to that "rank".
template<size_t... Maxes>
constexpr size_t vv_extract_index(size_t Combined, size_t Rank, index_sequence<Maxes...>)
{
    constexpr size_t maxes[] = { Maxes... };
    size_t i = sizeof...(Maxes);
    while (--i > Rank) {
        Combined /= maxes[i];
    }
    return Combined % maxes[i];
}

static_assert(vv_extract_index(123, 0, index_sequence<10,10,10>{}) == 1);
static_assert(vv_extract_index(123, 1, index_sequence<10,10,10>{}) == 2);
static_assert(vv_extract_index(123, 2, index_sequence<10,10,10>{}) == 3);
static_assert(vv_extract_index(22, 0, index_sequence<2,3,4>{}) == 1);
static_assert(vv_extract_index(22, 1, index_sequence<2,3,4>{}) == 2);
static_assert(vv_extract_index(22, 2, index_sequence<2,3,4>{}) == 2);


// Here is the actual implementation of "visit".

template<size_t Combined, size_t... Ranks, class Visitor, class... Variants>
decltype(auto) vvisit_known_index_impl(index_sequence<Ranks...>, Visitor&& f, Variants&&... vs)
{
    using MaxList = index_sequence<variant_size_v<remove_reference_t<Variants>>...>;
    return invoke(
        std::forward<Visitor>(f),
        scratch::get<vv_extract_index(Combined, Ranks, MaxList{})>(std::forward<Variants>(vs))...
    );
}

template<size_t Combined, class Visitor, class... Variants>
decltype(auto) vvisit_known_index(Visitor&& f, Variants&&... vs)
{
    using RankList = make_index_sequence<sizeof...(Variants)>;
    return vvisit_known_index_impl<Combined>(RankList{}, std::forward<Visitor>(f), std::forward<Variants>(vs)...);
}

template<size_t... Combineds, class Visitor, class... Variants>
decltype(auto) vvisit_impl(index_sequence<Combineds...>, Visitor&& f, Variants&&... vs)
{
    using Action = decltype(&vvisit_known_index<0, Visitor, Variants...>);
    static constexpr Action actions[] = {
        vvisit_known_index<Combineds, Visitor, Variants...> ...
    };
    const size_t combined_index = vv_combine_indices(
        index_sequence<variant_size_v<remove_reference_t<Variants>>...>{},
        vs.index()...
    );
    return actions[combined_index](
        std::forward<Visitor>(f),
        std::forward<Variants>(vs)...
    );
}

} // namespace scratch::detail

namespace scratch {

template<typename Visitor, typename... Variants>
decltype(auto) visit(Visitor&& f, Variants&&... vs)
{
    if ((vs.valueless_by_exception() || ...)) {
        throw bad_variant_access();
    }
    constexpr size_t MaxCombined = (size_t(1) * ... * variant_size_v<remove_reference_t<Variants>>);
    return detail::vvisit_impl(
        make_index_sequence<MaxCombined>(),
        std::forward<Visitor>(f),
        std::forward<Variants>(vs)...
    );
}

} // namespace scratch
