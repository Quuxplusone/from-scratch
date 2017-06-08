#pragma once

#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/variant/in-place-index.h"
#include "scratch/bits/variant/recursive-union.h"

#include <cstddef>
#include <new>
#include <utility>

namespace scratch {

inline constexpr size_t variant_npos = size_t(-1);

template<typename... Ts>
class variant {
    static_assert(sizeof...(Ts) >= 1, "variant must contain at least one alternative");
    using index_type = size_t;

    recursive_union<0, Ts...> m_union;
    index_type m_index;
public:
    size_t index() const noexcept { return m_index; }
    bool valueless_by_exception() const noexcept { return (m_index == variant_npos); }

    template<typename J> decltype(auto) get_element(J j) { return m_union.get_element(j); }
    template<typename J> decltype(auto) get_element(J j) const { return m_union.get_element(j); }

    constexpr variant() : m_union(index_constant<0>{}), m_index(0) {}

    template<size_t I, class... Args>
    constexpr variant(in_place_index_t<I>, Args&&... args) : m_union(index_constant<I>{}, std::forward<Args>(args)...), m_index(I) {}

    template<size_t Index, typename... Args>
    void emplace(Args&&... args) {
        reset();
        m_union.visit(index_constant<Index>(), [&](auto&& t){
            using T = decay_t<decltype(t)>;
            ::new (static_cast<void *>(&t)) T(std::forward<Args>(args)...);
        });
        m_index = Index;
    }

    ~variant() {
        reset();
    }

private:
    void reset() {
        if (!valueless_by_exception()) {
            m_union.visit(index(), [](auto&& t){
                using T = decay_t<decltype(t)>;
                t.~T();
            });
        }
    }
};

} // namespace scratch
