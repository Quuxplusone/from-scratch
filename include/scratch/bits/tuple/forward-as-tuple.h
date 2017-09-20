#pragma once

#include "scratch/bits/tuple/reftuple.h"

#include <utility>

namespace scratch {

template<class... Ts>
auto forward_as_tuple(Ts&&... ts)
{
    return reftuple<Ts&&...>(std::forward<Ts>(ts)...);
}

} // namespace scratch
