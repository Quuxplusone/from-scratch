#pragma once

#include <exception>

namespace scratch {

template<class E>
std::exception_ptr make_exception_ptr(E&& e) noexcept
{
    try {
        throw std::forward<E>(e);
    } catch (...) {
        return std::current_exception();
    }
}

} // namespace scratch
