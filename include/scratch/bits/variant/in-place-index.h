#pragma once

#include <cstddef>

namespace scratch {

template<size_t>
struct in_place_index_t {
    explicit in_place_index_t() = default;
};

template <size_t I> inline constexpr in_place_index_t<I> in_place_index{};

} // namespace scratch
