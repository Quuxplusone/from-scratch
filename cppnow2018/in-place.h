#pragma once

namespace scratch {

struct in_place_t {
    constexpr explicit in_place_t() = default;
};

constexpr in_place_t in_place{};

} // namespace scratch
