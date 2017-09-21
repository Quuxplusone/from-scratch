#pragma once

#include <tuple>
#include <cstddef>
#include <utility>
#include "index-of-tuple.h"
#include "index-into-tuple.h"

template<size_t... Sizes, class T, size_t... FlatIndices>
static auto result_impl(T reftuple_of_tuples, std::index_sequence<FlatIndices...>) {
    using ResultType = std::tuple<
        std::tuple_element_t<index_into_tuple<Sizes...>(FlatIndices),
            std::remove_reference_t<std::tuple_element_t<index_of_tuple<Sizes...>(FlatIndices),
                T
            >>
        >...
    >;
    return ResultType(
        std::get<index_into_tuple<Sizes...>(FlatIndices)>(
            std::get<index_of_tuple<Sizes...>(FlatIndices)>(
               reftuple_of_tuples
            )
        )...
    );
}


namespace my {

template<size_t... Sizes, class... Tuples>
auto tuple_cat_impl(Tuples&&... tuples) {
    using FlatIndexList = std::make_index_sequence<(0 + ... + Sizes)>;
    auto reftuple_of_tuples = std::forward_as_tuple(std::forward<Tuples>(tuples)...);
    return result_impl<Sizes...>(reftuple_of_tuples, FlatIndexList{});
}

template<class... Tuples>
auto tuple_cat(Tuples&&... tuples)
{
    return tuple_cat_impl<std::tuple_size_v<std::decay_t<Tuples>>... >(std::forward<Tuples>(tuples)...);
}

} // namespace my
