#pragma once

#include "scratch/bits/concurrency/future-and-promise.h"
#include "scratch/bits/smart-ptrs/shared-ptr.h"
#include "scratch/bits/tuple/tuple.h"
#include "scratch/bits/type-traits/decay.h"

#include <atomic>
#include <utility>

namespace scratch::detail {

template<class... Futures>
struct when_any_shared_state {
    promise<tuple<Futures...>> m_promise;
    tuple<Futures...> m_tuple;
    std::atomic<bool> m_done;
    std::atomic<bool> m_count_to_two;

    when_any_shared_state(promise<tuple<Futures...>> p) :
        m_promise(std::move(p)), m_done(false), m_count_to_two(false) {}
};

} // namespace scratch::detail

namespace scratch {

template<class... Futures>
auto when_any(Futures... futures) -> future<tuple<Futures...>>
{
    using shared_state = detail::when_any_shared_state<Futures...>;
    using R = tuple<Futures...>;
    promise<R> p;
    future<R> result = p.get_future();

    auto sptr = make_shared<shared_state>(std::move(p));
    auto satisfy_combined_promise =
        [sptr](auto f) {
            if (sptr->m_done.exchange(true) == false) {
                if (sptr->m_count_to_two.exchange(true)) {
                    sptr->m_promise.set_value(std::move(sptr->m_tuple));
                }
            }
            return f.get();
        };
    sptr->m_tuple = tuple<Futures...>(futures.then(satisfy_combined_promise)...);
    if (sptr->m_count_to_two.exchange(true)) {
        sptr->m_promise.set_value(std::move(sptr->m_tuple));
    }
    return result;
}

} // namespace scratch
