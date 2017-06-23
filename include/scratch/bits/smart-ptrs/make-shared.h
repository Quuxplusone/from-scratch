#pragma once

#include "scratch/bits/smart-ptrs/shared-ptr.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-base-of.h"
#include "scratch/bits/type-traits/remove-foo.h"

#include <utility>

namespace scratch::detail {

template<class T>
void maybe_enable_sharing_from_this(const shared_ptr<T>& sptr, false_type) {}

template<class T>
void maybe_enable_sharing_from_this(const shared_ptr<T>& sptr, true_type)
{
    sptr->enable_sharing_from_this(sptr);
}

} // namespace scratch::detail

namespace scratch {

template<typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args)
{
    auto result = shared_ptr<T>(new T(std::forward<Args>(args)...));
    using U = remove_cv_t<T>;
    detail::maybe_enable_sharing_from_this(result, is_base_of_v<enable_shared_from_this<U>, U>);
    return result;
}

} // namespace scratch
