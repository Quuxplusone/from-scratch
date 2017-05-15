#pragma once

namespace scratch {

struct adopt_lock_t { explicit adopt_lock_t() = default; };
struct defer_lock_t { explicit defer_lock_t() = default; };
struct try_to_lock_t { explicit try_to_lock_t() = default; };

inline constexpr adopt_lock_t adopt_lock{};
inline constexpr defer_lock_t defer_lock{};
inline constexpr try_to_lock_t try_to_lock{};

} // namespace scratch
