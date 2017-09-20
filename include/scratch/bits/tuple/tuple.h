#pragma once

#include "scratch/bits/tuple/tuple-size.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/remove-foo.h"
#include "scratch/bits/utility/compressed-element.h"
#include "scratch/bits/utility/declval.h"
#include "scratch/bits/utility/integer-sequence.h"

#include <utility>

namespace scratch {

template<class...> class tuple;

template<size_t I, class... Ts> constexpr inline decltype(auto) get(tuple<Ts...>& t) { return t.get_element(index_constant<I>{}); }
template<size_t I, class... Ts> constexpr inline decltype(auto) get(tuple<Ts...>&& t) { return std::move(t).get_element(index_constant<I>{}); }
template<size_t I, class... Ts> constexpr inline decltype(auto) get(const tuple<Ts...>& t) { return t.get_element(index_constant<I>{}); }
template<size_t I, class... Ts> constexpr inline decltype(auto) get(const tuple<Ts...>&& t) { return std::move(t).get_element(index_constant<I>{}); }

template<class, class...> struct tuple_impl;

template<class... Ts, size_t... Is>
struct tuple_impl<index_sequence<Is...>, Ts...> :
    private compressed_element<Ts, Is>...
{
    using ISeq = index_sequence<Is...>;
    using compressed_element<Ts, Is>::get_element...;

    void get_element() {}  // make sure there's something for reftuple's "using" declaration even in the zero-Ts case

    constexpr tuple_impl() = default;

    template<class... Us>
    tuple_impl(Us&&... args) : compressed_element<Ts, Is>(std::forward<Us>(args))... {}

    void swap(tuple_impl& rhs) {
        int dummy[] = {
            [&](){
                using std::swap;
                swap(scratch::get<Is>(*this), scratch::get<Is>(rhs));
                return 0;
            }()...
        };
    }

    template<class... Us>
    void assign(const tuple_impl<ISeq, Us...>& rhs) {
        int dummy[] = {
            [&](){
                scratch::get<Is>(*this) = scratch::get<Is>(rhs);
                return 0;
            }()...
        };
    }

    template<class... Us>
    void assign(tuple_impl<ISeq, Us...>&& rhs) {
        int dummy[] = {
            [&](){
                scratch::get<Is>(*this) = scratch::get<Is>(std::move(rhs));
                return 0;
            }()...
        };
    }
};

template<class... Ts>
class tuple : private tuple_impl<make_index_sequence<sizeof...(Ts)>, Ts...>
{
    static_assert((is_object_v<Ts> && ...), "Our tuple can hold only object types. For a tuple of references, use reftuple.");
    using TupleImpl = tuple_impl<make_index_sequence<sizeof...(Ts)>, Ts...>;
public:
    using TupleImpl::get_element;  // public

    constexpr tuple() = default;

    template<class... Us, class = enable_if_t<sizeof...(Us) == sizeof...(Ts)>>
    constexpr tuple(Us&&... args) : TupleImpl(std::forward<Us>(args)...) {}

    void swap(tuple& rhs) {
        TupleImpl::swap(rhs);
    }
};

template<class... Ts>
void swap(tuple<Ts...>& a, tuple<Ts...>& b)
{
    a.swap(b);
}

template<class... Ts>
struct tuple_size<tuple<Ts...>> : index_constant<sizeof...(Ts)> {};

template<class... Ts, size_t I>
struct tuple_element<tuple<Ts...>, I> {
    using type = remove_reference_t<decltype(scratch::get<I>(declval<tuple<Ts...>>()))>;
};

} // namespace scratch
