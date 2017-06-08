#pragma once

#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-base-of.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-function.h"
#include "scratch/bits/type-traits/priority-tag.h"
#include "scratch/bits/utility/declval.h"
#include "scratch/bits/utility/is-reference-wrapper.h"

#include <utility>

// If "f" is a pointer to member function of some class B:
// - If std::is_base_of_v<B, std::decay_t<O>>, then INVOKE(f, o, args...) is equivalent to (o.*f)(args...)
// - Otherwise, if std::decay_t<O> is a specialization of std::reference_wrapper,
//   then INVOKE(f, o, args...) is equivalent to (o.get().*f)(args...)
// - Otherwise, INVOKE(f, o, args...) is equivalent to ((*o).*f)(args...)
// Otherwise, if "f" is a pointer to data member of class B:
// - If std::is_base_of_v<B, std::decay_t<O>>, then INVOKE(f, o) is equivalent to (o.*f)
// - Otherwise, if std::decay_t<O> is a specialization of std::reference_wrapper,
//   then INVOKE(f, o) is equivalent to (o.get().*f)
// - Otherwise, INVOKE(f, o) is equivalent to ((*o).*f)
// Otherwise, if "f" is any other type, then INVOKE(f, args...) is equivalent to f(args...)

namespace scratch::detail {

// This is the "F is a member object pointer, O is reference to B object" case.
template<class B, class M, class O>
constexpr auto invoke_pm_impl(false_type, priority_tag<2>, M B::*f, O&& o)
    noexcept(noexcept( (std::forward<O>(o).*f) ))
    -> enable_if_t<is_base_of_v<B, decay_t<O>>,
        decltype( (std::forward<O>(o).*f) )>
{
    return (std::forward<O>(o).*f);
}

// This is the "F is a member object pointer, O is reference_wrapper" case.
template<class B, class M, class O>
constexpr auto invoke_pm_impl(false_type, priority_tag<1>, M B::*f, O&& o)
    noexcept(noexcept( (std::forward<O>(o).get().*f) ))
    -> enable_if_t<is_reference_wrapper_v<decay_t<O>>,
        decltype( (std::forward<O>(o).get().*f) )>
{
    return (std::forward<O>(o).get().*f);
}

// This is the "F is a member object pointer, O is (smart) pointer" case.
template<class B, class M, class O>
constexpr auto invoke_pm_impl(false_type, priority_tag<0>, M B::*f, O&& o)
    noexcept(noexcept( ((*std::forward<O>(o)).*f) ))
    -> decltype( ((*std::forward<O>(o)).*f) )
{
    return ((*std::forward<O>(o)).*f);
}

// This is the "F is a member function pointer, O is reference to B object" case.
template<class B, class M, class O, class... Args>
constexpr auto invoke_pm_impl(true_type, priority_tag<2>, M B::*f, O&& o, Args&&... args)
    noexcept(noexcept( (std::forward<O>(o).*f)(std::forward<Args>(args)...) ))
    -> enable_if_t<is_base_of_v<B, decay_t<O>>,
        decltype( (std::forward<O>(o).*f)(std::forward<Args>(args)...) )>
{
    return (std::forward<O>(o).*f)(std::forward<Args>(args)...);
}

// This is the "F is a member function pointer, O is reference_wrapper" case.
template<class B, class M, class O, class... Args>
constexpr auto invoke_pm_impl(true_type, priority_tag<1>, M B::*f, O&& o, Args&&... args)
    noexcept(noexcept( (std::forward<O>(o).get().*f)(std::forward<Args>(args)...) ))
    -> enable_if_t<is_reference_wrapper_v<decay_t<O>>,
        decltype( (std::forward<O>(o).get().*f)(std::forward<Args>(args)...) )>
{
    return (std::forward<O>(o).get().*f)(std::forward<Args>(args)...);
}

// This is the "F is a member function pointer, O is (smart) pointer" case.
template<class B, class M, class O, class... Args>
constexpr auto invoke_pm_impl(true_type, priority_tag<0>, M B::*f, O&& o, Args&&... args)
    noexcept(noexcept( ((*std::forward<O>(o)).*f)(std::forward<Args>(args)...) ))
    -> decltype( ((*std::forward<O>(o)).*f)(std::forward<Args>(args)...) )
{
    return ((*std::forward<O>(o)).*f)(std::forward<Args>(args)...);
}

// This is the "F is a member pointer" case.
template<class B, class M, class O, class... Args>
constexpr auto invoke_impl(true_type, M B::*f, O&& o, Args&&... args)
    noexcept(noexcept( invoke_pm_impl(is_function<M>{}, priority_tag<2>{}, f, std::forward<O>(o), std::forward<Args>(args)...) ))
    -> decltype( invoke_pm_impl(is_function<M>{}, priority_tag<2>{}, f, std::forward<O>(o), std::forward<Args>(args)...) )
{
    return invoke_pm_impl(is_function<M>{}, priority_tag<2>{}, f, std::forward<O>(o), std::forward<Args>(args)...);
}

// This is the "not a member pointer" case. It's easy.
template<class F, class... Args>
constexpr auto invoke_impl(false_type, F&& f, Args&&... args)
    noexcept(noexcept( std::forward<F>(f)(std::forward<Args>(args)...) ))
    -> decltype( std::forward<F>(f)(std::forward<Args>(args)...) )
{
    return std::forward<F>(f)(std::forward<Args>(args)...);
}

} // namespace scratch::detail

namespace scratch {

template<class F, class... Args>
constexpr auto invoke(F&& f, Args&&... args)
    noexcept(noexcept( detail::invoke_impl(is_member_pointer<decay_t<F>>{}, std::forward<F>(f), std::forward<Args>(args)...) ))
    -> decltype( detail::invoke_impl(is_member_pointer<decay_t<F>>{}, std::forward<F>(f), std::forward<Args>(args)...) )
{
    return detail::invoke_impl(is_member_pointer<decay_t<F>>{}, std::forward<F>(f), std::forward<Args>(args)...);
}

} // namespace scratch


namespace scratch::detail {

template<class Enable, class... Ts> struct is_invocable_impl : false_type {};
template<class... Ts>               struct is_invocable_impl<decltype(void(scratch::invoke(declval<Ts>()...))), Ts...> : true_type {};

} // namespace scratch::detail

namespace scratch {

template<class... Ts> struct is_invocable : detail::is_invocable_impl<void, Ts...> {};
template<class... Ts> inline constexpr bool is_invocable_v = is_invocable<Ts...>::value;

} // namespace scratch


namespace scratch::detail {

template<class Enable, class... Ts> struct invoke_result_impl {};
template<class... Ts>               struct invoke_result_impl<std::enable_if_t<is_invocable_v<Ts...>>, Ts...> {
    using type = decltype(scratch::invoke(declval<Ts>()...));
};

} // namespace scratch::detail

namespace scratch {

template<class... Ts> struct invoke_result : detail::invoke_result_impl<void, Ts...> {};
template<class... Ts> using invoke_result_t = typename invoke_result<Ts...>::type;

} // namespace scratch
