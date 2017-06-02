#pragma once

#include "scratch/bits/traits-classes/iterator-traits.h"
#include "scratch/bits/type-traits/add-foo.h"
#include "scratch/bits/type-traits/conditional.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/remove-foo.h"

#include <utility>

namespace scratch {

template<class It>
struct move_iterator
{
    using value_type = typename iterator_traits<It>::value_type;
    using difference_type = typename iterator_traits<It>::difference_type;
    using pointer = It;
    using reference = conditional_t<
        is_reference_v<typename iterator_traits<It>::reference>,
        add_rvalue_reference_t<remove_reference_t<typename iterator_traits<It>::reference>>,
        typename iterator_traits<It>::reference
    >;
    using iterator_category = typename iterator_traits<It>::iterator_category;

    using iterator_type = It;

    constexpr move_iterator() = default;
    constexpr explicit move_iterator(It x) : m_base(std::move(x)) {}

    // Notice that this is also our copy constructor.
    template<class U>
    constexpr move_iterator(const move_iterator<U>& u) : m_base(u.base()) {}

    // Notice that this is also our copy assignment operator.
    template<class U>
    constexpr move_iterator& operator=(const move_iterator<U>& u) {
        m_base = u.base();
        return *this;
    }

    constexpr It base() const { return m_base; }

    constexpr reference operator*() const { return static_cast<reference>(*base()); }
    constexpr pointer operator->() const { return base(); }

    constexpr reference operator[](difference_type n) const { return static_cast<reference>(*(base() + n)); }

    constexpr move_iterator& operator++() { ++m_base; return *this; }
    constexpr move_iterator& operator++(int) { auto result = *this; ++m_base; return result; }
    constexpr move_iterator& operator--() { --m_base; return *this; }
    constexpr move_iterator& operator--(int) { auto result = *this; --m_base; return result; }

    constexpr move_iterator& operator+=(difference_type n) { m_base += n; return *this; }
    constexpr move_iterator& operator-=(difference_type n) { m_base -= n; return *this; }

private:
    // Unlike reverse_iterator's, this member is indeed private in the standard library.
    It m_base;
};

template<class It> constexpr move_iterator<It> operator+(move_iterator<It> r, iterator_difference_t<It> n) { return r += n; }
template<class It> constexpr move_iterator<It> operator-(move_iterator<It> r, iterator_difference_t<It> n) { return r -= n; }
template<class It> constexpr move_iterator<It> operator+(iterator_difference_t<It> n, move_iterator<It> r) { return r += n; }
template<class It> constexpr iterator_difference_t<It> operator-(move_iterator<It> a, move_iterator<It> b) { return a.base() - b.base(); }

template<class A, class B> constexpr bool operator==(const move_iterator<A>& a, const move_iterator<B>& b) { return a.base() == b.base(); }
template<class A, class B> constexpr bool operator!=(const move_iterator<A>& a, const move_iterator<B>& b) { return a.base() != b.base(); }
template<class A, class B> constexpr bool operator<(const move_iterator<A>& a, const move_iterator<B>& b) { return a.base() < b.base(); }
template<class A, class B> constexpr bool operator<=(const move_iterator<A>& a, const move_iterator<B>& b) { return a.base() <= b.base(); }
template<class A, class B> constexpr bool operator>(const move_iterator<A>& a, const move_iterator<B>& b) { return a.base() > b.base(); }
template<class A, class B> constexpr bool operator>=(const move_iterator<A>& a, const move_iterator<B>& b) { return a.base() >= b.base(); }

} // namespace scratch
