#pragma once

#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/concurrency/future-and-promise.h"

#include <exception>
#include <utility>

namespace scratch {

template<class T, class... Args>
auto make_ready_future(Args&&... args)
{
    promise<T> promise;
    promise.set_value(T(std::forward<Args>(args)...));
    return promise.get_future();
}

template<class T>
auto make_ready_future(T t)
{
    promise<T> promise;
    promise.set_value(std::move(t));
    return promise.get_future();
}

template<class T>
auto make_exceptional_future(std::exception_ptr e)
{
    promise<T> promise;
    promise.set_exception(std::move(e));
    return promise.get_future();
}

} // namespace scratch
