#pragma once

#include "scratch/bits/tuple/tuple-size.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/remove-foo.h"
#include "scratch/bits/utility/compressed-pair.h"
#include "scratch/bits/utility/declval.h"
#include "scratch/bits/utility/integer-sequence.h"

#include <utility>

namespace scratch::detail {

template<typename T, size_t Index>
class compressed_ref_element {
    static_assert(is_reference_v<T>);
    remove_reference_t<T> *t;
public:
    template<class U> compressed_ref_element(U&& u) : t(&u) {}
    T get() const { return static_cast<T>(*t); }
    T get_element(index_constant<Index>) const { return static_cast<T>(*t); }
};

} // namespace scratch::detail

namespace scratch {

template<class...> class reftuple;

template<size_t I, class... Ts> constexpr inline decltype(auto) get(const reftuple<Ts...>& t) { return t.get_element(index_constant<I>{}); }

template<class, class...> struct reftuple_impl;

template<class... Ts, size_t... Is>
struct reftuple_impl<index_sequence<Is...>, Ts...> :
    private detail::compressed_ref_element<Ts, Is>...
{
    using ISeq = index_sequence<Is...>;
    using detail::compressed_ref_element<Ts, Is>::get_element...;

    void get_element() {}  // make sure there's something for reftuple's "using" declaration even in the zero-Ts case

    template<class... Us>
    reftuple_impl(Us&&... args) : detail::compressed_ref_element<Ts, Is>(std::forward<Us>(args))... {}
};

template<class... Ts>
class reftuple : private reftuple_impl<make_index_sequence<sizeof...(Ts)>, Ts...>
{
    static_assert((is_reference_v<Ts> && ...), "reftuple can hold only reference types. For a tuple of object types, use tuple.");
    using TupleImpl = reftuple_impl<make_index_sequence<sizeof...(Ts)>, Ts...>;
public:
    using TupleImpl::get_element;  // public

    reftuple(reftuple&&) = default;
    reftuple(const reftuple&) = default;
    reftuple& operator=(reftuple&&) = delete;
    reftuple& operator=(const reftuple&) = delete;
    ~reftuple() = default;

    template<class... Us, class = enable_if_t<sizeof...(Us) == sizeof...(Ts)>>
    constexpr reftuple(Us&&... args) : TupleImpl(std::forward<Us>(args)...) {}
};

template<class... Ts>
struct tuple_size<reftuple<Ts...>> : index_constant<sizeof...(Ts)> {};

template<class... Ts, size_t I>
struct tuple_element<reftuple<Ts...>, I> {
    using type = decltype(scratch::get<I>(declval<reftuple<Ts...>>()));
};

} // namespace scratch
