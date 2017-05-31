#pragma once

#include "scratch/bits/type-traits/priority-tag.h"
#include "scratch/bits/type-traits/remove-foo.h"
#include "scratch/bits/utility/declval.h"

namespace scratch::detail {

template<class U, class T>
auto pointer_rebind(T *&, priority_tag<2>) -> U *;

template<class U, class Ptr>
auto pointer_rebind(Ptr&, priority_tag<1>) -> typename Ptr::template rebind<U>;

template<class U, template<class...> class SomePointer, class T, class... Vs>
auto pointer_rebind(SomePointer<T, Vs...>&, priority_tag<0>) -> SomePointer<U, Vs...>;

} // namespace scratch::detail

namespace scratch {

template<class Ptr>
struct pointer_traits
{
    using pointer = Ptr;
    using element_type = remove_reference_t<decltype(*declval<Ptr>())>;
    using difference_type = remove_reference_t<decltype(declval<Ptr>() - declval<Ptr>())>;

    template<class U> using rebind =
        decltype(detail::pointer_rebind<U>(declval<Ptr&>(), priority_tag<2>{}));
};

} // namespace scratch
