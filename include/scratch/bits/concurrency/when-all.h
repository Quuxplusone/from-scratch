#pragma once

#include "scratch/bits/concurrency/future-and-promise.h"
#include "scratch/bits/concurrency/make-ready-future.h"
#include "scratch/bits/tuple/make-tuple.h"
#include "scratch/bits/tuple/tuple.h"
#include "scratch/bits/tuple/tuple-cat.h"
#include "scratch/bits/type-traits/decay.h"

#include <utility>

namespace scratch {

auto when_all() -> future<tuple<>>
{
    return make_ready_future<tuple<>>();
}

template<class Head, class... Tails>
auto when_all(Head&& head, Tails&&... tails) -> future<tuple<decay_t<Head>, decay_t<Tails>...>>
{
    auto wait_on_tail = scratch::when_all(std::move(tails)...);

    return head.then([t = std::move(wait_on_tail)](auto h) mutable {
        return tuple_cat(make_tuple(std::move(h)), t.get());
    });
}

} // namespace scratch
