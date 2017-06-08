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

template<size_t Index, typename Visitor, typename Variant>
decltype(auto) visit_known_index(Visitor&& f, Variant&& v)
{
    if constexpr (Index == size_t(-1)) {
        throw bad_variant_access();
    } else {
        return invoke(std::forward<Visitor>(f), scratch::get<Index>(std::forward<Variant>(v)));
    }
}

template<typename Visitor, typename Variant, size_t... Is>
decltype(auto) visit_impl(Visitor&& f, Variant&& v, index_sequence<Is...>)
{
    using Action = decltype(&visit_known_index<0, Visitor, Variant>);
    Action actions[] = {
        visit_known_index<size_t(-1), Visitor, Variant>,
        visit_known_index<Is, Visitor, Variant> ...
    };
    return actions[v.index() + 1](
        std::forward<Visitor>(f),
        std::forward<Variant>(v)
    );
}

} // namespace scratch::detail

namespace scratch {

template<typename Visitor, typename Variant>
decltype(auto) visit(Visitor&& f, Variant&& v)
{
    return detail::visit_impl(
        std::forward<Visitor>(f),
        std::forward<Variant>(v),
        make_index_sequence<variant_size_v<remove_reference_t<Variant>>>()
    );
}

} // namespace scratch
