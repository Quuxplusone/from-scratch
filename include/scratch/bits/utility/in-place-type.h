#pragma once

namespace scratch {

template<typename>
struct in_place_type_t {
    explicit in_place_type_t() = default;
};

template <typename T> inline constexpr in_place_type_t<T> in_place_type{};

} // namespace scratch
