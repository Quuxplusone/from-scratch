#pragma once

#include "scratch/bits/traits-classes/pointer-traits.h"
#include "scratch/bits/type-traits/compiler-magic.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-signed.h"
#include "scratch/bits/type-traits/priority-tag.h"
#include "scratch/bits/type-traits/remove-foo.h"

#include <new>
#include <utility>

namespace scratch::detail {

template<class Alloc> auto allocator_value_type(priority_tag<1>) -> typename Alloc::value_type*;
template<class Alloc> auto allocator_value_type(priority_tag<0>) -> decltype(*declval<Alloc&>().allocate(0))*;
template<class Alloc> using allocator_value_type_t = remove_pointer_t<decltype(allocator_value_type<Alloc>(priority_tag<1>{}))>;

template<class Alloc> auto allocator_pointer(priority_tag<2>) -> typename Alloc::pointer;
template<class Alloc> auto allocator_pointer(priority_tag<1>) -> decltype(declval<Alloc&>().allocate(0));
template<class Alloc> auto allocator_pointer(priority_tag<0>) -> allocator_value_type_t<Alloc>*;
template<class Alloc> using allocator_pointer_t = decltype(allocator_pointer<Alloc>(priority_tag<2>{}));

template<class Alloc> auto allocator_const_pointer(priority_tag<1>) -> typename Alloc::const_pointer;
template<class Alloc> auto allocator_const_pointer(priority_tag<0>) -> typename pointer_traits<allocator_pointer_t<Alloc>>::template rebind<const allocator_value_type_t<Alloc>>;
template<class Alloc> using allocator_const_pointer_t = decltype(allocator_const_pointer<Alloc>(priority_tag<1>{}));

template<class Alloc> auto allocator_void_pointer(priority_tag<1>) -> typename Alloc::void_pointer;
template<class Alloc> auto allocator_void_pointer(priority_tag<0>) -> typename pointer_traits<allocator_pointer_t<Alloc>>::template rebind<void>;
template<class Alloc> using allocator_void_pointer_t = decltype(allocator_void_pointer<Alloc>(priority_tag<1>{}));

template<class Alloc> auto allocator_const_void_pointer(priority_tag<1>) -> typename Alloc::const_void_pointer;
template<class Alloc> auto allocator_const_void_pointer(priority_tag<0>) -> typename pointer_traits<allocator_pointer_t<Alloc>>::template rebind<const void>;
template<class Alloc> using allocator_const_void_pointer_t = decltype(allocator_const_void_pointer<Alloc>(priority_tag<1>{}));

template<class Alloc> auto allocator_difference_type(priority_tag<1>) -> typename Alloc::difference_type;
template<class Alloc> auto allocator_difference_type(priority_tag<0>) -> typename pointer_traits<allocator_pointer_t<Alloc>>::difference_type;
template<class Alloc> using allocator_difference_type_t = decltype(allocator_difference_type<Alloc>(priority_tag<1>{}));

template<class Alloc> auto allocator_size_type(priority_tag<1>) -> typename Alloc::size_type;
template<class Alloc> auto allocator_size_type(priority_tag<0>) -> make_unsigned_t<typename pointer_traits<allocator_pointer_t<Alloc>>::difference_type>;
template<class Alloc> using allocator_size_type_t = decltype(allocator_size_type<Alloc>(priority_tag<1>{}));

template<class Alloc> auto allocator_pocca(priority_tag<1>) -> typename Alloc::propagate_on_container_copy_assignment;
template<class Alloc> auto allocator_pocca(priority_tag<0>) -> false_type;

template<class Alloc> auto allocator_pocma(priority_tag<1>) -> typename Alloc::propagate_on_container_move_assignment;
template<class Alloc> auto allocator_pocma(priority_tag<0>) -> false_type;

template<class Alloc> auto allocator_pocs(priority_tag<1>) -> typename Alloc::propagate_on_container_swap;
template<class Alloc> auto allocator_pocs(priority_tag<0>) -> false_type;

template<class Alloc> auto allocator_iae(priority_tag<1>) -> typename Alloc::is_always_equal;
template<class Alloc> auto allocator_iae(priority_tag<0>) -> typename is_empty<Alloc>::type;

template<class U, class Alloc>
auto allocator_rebind(Alloc&, priority_tag<1>) -> typename Alloc::template rebind<U>;
template<class U, template<class...> class SomeAllocator, class T, class... Vs>
auto allocator_rebind(SomeAllocator<T, Vs...>&, priority_tag<0>) -> SomeAllocator<U, Vs...>;

} // namespace scratch::detail

namespace scratch {

template<class Alloc>
struct allocator_traits {
    using allocator_type = Alloc;
    using value_type = detail::allocator_value_type_t<Alloc>;

    using pointer = detail::allocator_pointer_t<Alloc>;
    using const_pointer = detail::allocator_const_pointer_t<Alloc>;
    using void_pointer = detail::allocator_void_pointer_t<Alloc>;
    using const_void_pointer = detail::allocator_const_void_pointer_t<Alloc>;
    using difference_type = detail::allocator_difference_type_t<Alloc>;
    using size_type = detail::allocator_size_type_t<Alloc>;

    using propagate_on_container_copy_assignment = decltype(detail::allocator_pocca<Alloc>(priority_tag<1>{}));
    using propagate_on_container_move_assignment = decltype(detail::allocator_pocma<Alloc>(priority_tag<1>{}));
    using propagate_on_container_swap = decltype(detail::allocator_pocs<Alloc>(priority_tag<1>{}));
    using is_always_equal = decltype(detail::allocator_iae<Alloc>(priority_tag<1>{}));

    // These convenience aliases are NOT standard!
    static constexpr bool propagate_on_container_copy_assignment_v = propagate_on_container_copy_assignment{};
    static constexpr bool propagate_on_container_move_assignment_v = propagate_on_container_move_assignment{};
    static constexpr bool propagate_on_container_swap_v = propagate_on_container_swap{};
    static constexpr bool is_always_equal_v = is_always_equal{};

    template<class U> using rebind_alloc = decltype(detail::allocator_rebind<U>(declval<Alloc&>(), priority_tag<1>{}));
    template<class U> using rebind_traits = allocator_traits<rebind_alloc<U>>;

private:
    template<class Alloca>
    static auto soccc_impl(priority_tag<1>, const Alloca& a) -> decltype(a.select_on_container_copy_construction())
    { return a.select_on_container_copy_construction(); }

    static Alloc soccc_impl(priority_tag<0>, const Alloc& a)
    { return a; }

    template<class U, class... Args, class Alloca>
    static constexpr auto construct_impl(priority_tag<1>, Alloca& a, U *p, Args&&... args) -> decltype(a.construct(p, std::forward<Args>(args)...))
    { return a.construct(p, std::forward<Args>(args)...); }

    template<class U, class... Args>
    static void construct_impl(priority_tag<0>, Alloc&, U *p, Args&&... args)
    { ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...); }

    template<class U, class Alloca>
    static constexpr auto default_construct_impl(priority_tag<1>, Alloca& a, U *p) -> decltype(a.default_construct(p))
    { return a.default_construct(p); }

    template<class U>
    static void default_construct_impl(priority_tag<0>, Alloc&, U *p)
    { ::new (static_cast<void*>(p)) U; }

    template<class U, class Alloca>
    static constexpr auto destroy_impl(priority_tag<1>, Alloca& a, U *p) -> decltype(a.destroy(p))
    { return a.destroy(p); }

    template<class U>
    static constexpr void destroy_impl(priority_tag<0>, Alloc&, U *p)
    { p->~U(); }

    template<class Alloca>
    static constexpr auto max_size_impl(priority_tag<1>, const Alloca& a) -> decltype(a.max_size())
    { return a.max_size(); }

    static constexpr auto max_size_impl(priority_tag<0>, const Alloc&)
    { return size_type(-1) / sizeof (value_type); }

public:
    static constexpr pointer allocate(Alloc& a, size_type n) { return a.allocate(n); }
    static constexpr void deallocate(Alloc& a, pointer p, size_type n) { a.deallocate(p, n); }

    static constexpr Alloc select_on_container_copy_construction(const Alloc& a) {
        return soccc_impl(priority_tag<1>{}, a);
    }

    template<class... Args>
    static constexpr void construct(Alloc& a, value_type *p, Args&&... args) {
        construct_impl(priority_tag<1>{}, a, p, std::forward<Args>(args)...);
    }

    template<class U>
    static constexpr void default_construct(Alloc& a, U *p) {
        default_construct_impl(priority_tag<1>{}, a, p);
    }

    static constexpr void destroy(Alloc& a, value_type *p) {
        destroy_impl(priority_tag<1>{}, a, p);
    }

    static constexpr size_type max_size(const Alloc& a) {
        return max_size_impl(priority_tag<1>{}, a);
    }
};

} // namespace scratch
