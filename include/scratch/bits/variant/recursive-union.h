#pragma once

#include "scratch/bits/type-traits/integral-constant.h"

#include <cstddef>
#include <utility>

namespace scratch {

template<size_t, typename...>
class recursive_union;

template<size_t Index>
class recursive_union<Index> {
};

template<size_t Index, typename T, typename... Rest>
class recursive_union<Index, T, Rest...> {
    union {
        T m_value;
        recursive_union<Index+1, Rest...> m_rest;
    };

public:
    recursive_union() = default;

    template<typename... Args>
    recursive_union(index_constant<Index>, Args&&... args) : m_value(std::forward<Args>(args)...) {}

    template<size_t I, typename... Args>
    recursive_union(index_constant<I> i, Args&&... args) : m_rest(i, std::forward<Args>(args)...) {}

    ~recursive_union() {}  // use destroy() instead

    template<class F> decltype(auto) visit(index_constant<Index>, const F& f) & { return f(m_value); }
    template<class F> decltype(auto) visit(index_constant<Index>, const F& f) && { return f(std::move(m_value)); }
    template<class F> decltype(auto) visit(index_constant<Index>, const F& f) const & { return f(m_value); }
    template<class F> decltype(auto) visit(index_constant<Index>, const F& f) const && { return f(std::move(m_value)); }
    template<size_t I, class F> decltype(auto) visit(index_constant<I> i, const F& f) & { return m_rest.visit(i, f); }
    template<size_t I, class F> decltype(auto) visit(index_constant<I> i, const F& f) && { return std::move(m_rest).visit(i, f); }
    template<size_t I, class F> decltype(auto) visit(index_constant<I> i, const F& f) const & { return m_rest.visit(i, f); }
    template<size_t I, class F> decltype(auto) visit(index_constant<I> i, const F& f) const && { return std::move(m_rest).visit(i, f); }

    template<class F> decltype(auto) visit(size_t i, const F& f) & {
        if constexpr (sizeof...(Rest) != 0) {
            return (i == Index) ? f(m_value) : m_rest.visit(i, f);
        } else {
            return f(m_value);
        }
    }
    template<class F> decltype(auto) visit(size_t i, const F& f) && {
        if constexpr (sizeof...(Rest) != 0) {
            return (i == Index) ? f(std::move(m_value)) : std::move(m_rest).visit(i, f);
        } else {
            return f(std::move(m_value));
        }
    }
    template<class F> decltype(auto) visit(size_t i, const F& f) const & {
        if constexpr (sizeof...(Rest) != 0) {
            return (i == Index) ? f(m_value) : m_rest.visit(i, f);
        } else {
            return f(m_value);
        }
    }
    template<class F> decltype(auto) visit(size_t i, const F& f) const && {
        if constexpr (sizeof...(Rest) != 0) {
            return (i == Index) ? f(std::move(m_value)) : std::move(m_rest).visit(i, f);
        } else {
            return f(std::move(m_value));
        }
    }

    T& get_element(index_constant<Index>) { return m_value; }
    const T& get_element(index_constant<Index>) const { return m_value; }

    template<typename J> decltype(auto) get_element(J j) { return m_rest.get_element(j); }
    template<typename J> decltype(auto) get_element(J j) const { return m_rest.get_element(j); }
};

} // namespace scratch
