#pragma once

namespace scratch {

struct allocator_arg_t {
    constexpr explicit allocator_arg_t(int) {}
};

inline constexpr allocator_arg_t allocator_arg(42);

} // namespace scratch
