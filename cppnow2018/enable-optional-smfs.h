#pragma once

#include <type_traits>
#include "polyfill.h"

namespace scratch {

template<class T> class optional;  // forward declaration

} // namespace scratch

namespace scratch::detail {

using std::is_assignable_v;
using std::is_constructible_v;
using std::is_convertible_v;
using std::decay_t;
using std::is_same_v;
using std::is_scalar_v;

// This type-trait is "true" if T is somehow convertible-from optional<U>,
// which means that if optional<T> and optional<U> are on the left and
// right-hand sides of an assignment expression, we should prefer to
// interpret that as `optT.emplace(optU)` instead of
// `optT = (optU ? *optU : nullopt)`.

template<class T, class U>
constexpr bool enable_assignment_of_optional_from_forwarded_v =
    !is_same_v<decay_t<U>, optional<T>> && (
        !is_scalar_v<T> ||
        !is_same_v<decay_t<U>, T>
    );

template<class T, class U>
constexpr bool enable_assignment_of_optional_from_optional_v = !(
    is_constructible_v<T, optional<U>&>         ||
    is_constructible_v<T, const optional<U>&>   ||
    is_constructible_v<T, optional<U>&&>        ||
    is_constructible_v<T, const optional<U>&&>  ||
    is_convertible_v<optional<U>&, T>           ||
    is_convertible_v<const optional<U>&, T>     ||
    is_convertible_v<optional<U>&&, T>          ||
    is_convertible_v<const optional<U>&&, T>    ||
    is_assignable_v<T&, optional<U>&>           ||
    is_assignable_v<T&, const optional<U>&>     ||
    is_assignable_v<T&, optional<U>&&>          ||
    is_assignable_v<T&, const optional<U>&&>
);

} // namespace scratch::detail
