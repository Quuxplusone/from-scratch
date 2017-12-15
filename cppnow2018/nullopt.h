#pragma once

namespace scratch {

struct nullopt_t {
    constexpr explicit nullopt_t(int) {}
};

constexpr nullopt_t nullopt(42);

} // namespace scratch
