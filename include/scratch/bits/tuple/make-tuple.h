#pragma once

#include "scratch/bits/tuple/tuple.h"
#include "scratch/bits/type-traits/decay.h"

#include <utility>

namespace scratch {

template<class... Ts>
auto make_tuple(Ts&&... ts)
{
    return tuple<decay_t<Ts>...>(std::forward<Ts>(ts)...);
}

} // namespace scratch
