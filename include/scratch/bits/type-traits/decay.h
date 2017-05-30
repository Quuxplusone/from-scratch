#pragma once

#include "scratch/bits/type-traits/add-foo.h"
#include "scratch/bits/type-traits/conditional.h"
#include "scratch/bits/type-traits/extent.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-function.h"
#include "scratch/bits/type-traits/remove-foo.h"

namespace scratch::detail {

template<class T>
using decay_impl_t =
        conditional_t< 
            is_array_v<T>,
            remove_extent_t<T> *,
            conditional_t< 
                is_function_v<T>,
                add_pointer_t<T>,
                remove_cv_t<T>
            >
        >;

} // namespace scratch::detail

namespace scratch {

template<class T> struct decay { using type = detail::decay_impl_t<remove_reference_t<T>>; };

template<class T> using decay_t = typename decay<T>::type;

} // namespace scratch
