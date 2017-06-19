#pragma once

#include "scratch/bits/smart-ptrs/shared-ptr.h"

#include <utility>

namespace scratch {

template<typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args)
{
    return shared_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace scratch
