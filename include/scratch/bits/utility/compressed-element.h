#pragma once

#include "scratch/bits/type-traits/compiler-magic.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/remove-foo.h"

#include <cstddef>
#include <utility>

namespace scratch {

template<typename T, size_t Index, bool = is_empty_v<T> && !is_final_v<T>>
class compressed_element
{
    T t;
public:
    compressed_element() = default;

    template<typename U = T>
    compressed_element(U&& u) : t(std::forward<U>(u)) {}

    T& get() { return t; }
    const T& get() const { return t; }

    T& get_element(index_constant<Index>) & { return t; }
    T&& get_element(index_constant<Index>) && { return std::move(t); }
    const T& get_element(index_constant<Index>) const & { return t; }
    const T&& get_element(index_constant<Index>) const && { return std::move(t); }
};

template<typename T, size_t Index>
class compressed_element<T&&, Index, false>
{
    remove_reference_t<T> *t;
public:
    compressed_element(T&& u) : t(&u) {}

    T&& get() const { return t; }

    T&& get_element(index_constant<Index>) const { return static_cast<T&&>(*t); }
};

template<typename T, size_t Index>
class compressed_element<T, Index, true>
    : private T
{
public:
    compressed_element() = default;

    template<typename U = T>
    compressed_element(U&& u) : T(std::forward<U>(u)) {}

    T& get_element(index_constant<Index>) & { return *this; }
    T&& get_element(index_constant<Index>) && { return std::move(*this); }
    const T& get_element(index_constant<Index>) const & { return *this; }
    const T&& get_element(index_constant<Index>) const && { return std::move(*this); }
};

} // namespace scratch
