#pragma once

#include "scratch/bits/algorithm/make-heap.h"
#include "scratch/bits/containers/vector.h"
#include "scratch/bits/functional/less.h"
#include "scratch/bits/type-traits/is-swappable.h"
#include "scratch/bits/type-traits/uses-allocator.h"

#include <utility>

namespace scratch {

template<class T, class Container = vector<T>, class Compare = less<typename Container::value_type>>
class priority_queue {

protected:
    Container c;
    Compare comp;

public:
    using container_type = Container;
    using value_compare = Compare;
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;

    explicit priority_queue() : c(), comp() {}
    explicit priority_queue(const Compare& compare) : c(), comp(compare) {}
    explicit priority_queue(const Compare& compare, Container&& ctr) : c(std::move(ctr)), comp(compare) {
        scratch::make_heap(c.begin(), c.end(), comp);
    }
    priority_queue(const Compare& compare, const Container& ctr) : c(ctr), comp(compare) {
        scratch::make_heap(c.begin(), c.end(), comp);
    }

    template<class Alloc, class = enable_if<uses_allocator_v<Container, Alloc>>>
    explicit priority_queue(const Alloc& alloc) : c(alloc), comp() {}

    template<class Alloc, class = enable_if<uses_allocator_v<Container, Alloc>>>
    priority_queue(const Compare& compare, const Alloc& alloc) : c(alloc), comp(compare) {}

    template<class Alloc, class = enable_if<uses_allocator_v<Container, Alloc>>>
    priority_queue(const Compare& compare, const Container& ctr, const Alloc& alloc) : c(ctr, alloc), comp(compare) {
        scratch::make_heap(c.begin(), c.end(), comp);
    }

    template<class Alloc, class = enable_if<uses_allocator_v<Container, Alloc>>>
    priority_queue(const Compare& compare, Container&& ctr, const Alloc& alloc) : c(std::move(ctr), alloc), comp(compare) {
        scratch::make_heap(c.begin(), c.end(), comp);
    }

    template<class Alloc, class = enable_if<uses_allocator_v<Container, Alloc>>>
    priority_queue(const priority_queue& rhs, const Alloc& alloc) : c(rhs.c, alloc), comp(rhs.comp) {
        scratch::make_heap(c.begin(), c.end(), comp);
    }

    template<class Alloc, class = enable_if<uses_allocator_v<Container, Alloc>>>
    priority_queue(priority_queue&& rhs, const Alloc& alloc) : c(std::move(rhs.c), alloc), comp(std::move(rhs.comp)) {
        scratch::make_heap(c.begin(), c.end(), comp);
    }

    template<class InputIt>
    priority_queue(InputIt first, InputIt last, const Compare& compare, const Container& ctr) : c(ctr), comp(compare) {
        c.insert(c.end(), first, last);
        scratch::make_heap(c.begin(), c.end(), comp);
    }

    template<class InputIt>
    priority_queue(InputIt first, InputIt last) : c(), comp() {
        c.insert(c.end(), first, last);
        scratch::make_heap(c.begin(), c.end(), comp);
    }

    template<class InputIt>
    priority_queue(InputIt first, InputIt last, const Compare& compare) : c(), comp(compare) {
        c.insert(c.end(), first, last);
        scratch::make_heap(c.begin(), c.end(), comp);
    }

    template<class InputIt>
    priority_queue(InputIt first, InputIt last, const Compare& compare, Container&& ctr) : c(std::move(ctr)), comp(compare) {
        c.insert(c.end(), first, last);
        scratch::make_heap(c.begin(), c.end(), comp);
    }

    priority_queue(const priority_queue&) = default;
    priority_queue(priority_queue&&) = default;
    priority_queue& operator=(const priority_queue&) = default;
    priority_queue& operator=(priority_queue&&) = default;
    ~priority_queue() = default;

    void swap(priority_queue& rhs) noexcept(is_nothrow_swappable_v<Container> && is_nothrow_swappable_v<Compare>) {
        using std::swap;
        swap(c, rhs.c);
        swap(comp, rhs.comp);
    }

    const_reference top() const { return c.front(); }
    bool empty() const { return c.empty(); }
    bool size() const { return c.size(); }

    void push(const value_type& value) {
        c.push_back(value);
        scratch::push_heap(c.begin(), c.end(), comp);
    }

    void push(value_type&& value) {
        c.push_back(std::move(value));
        scratch::push_heap(c.begin(), c.end(), comp);
    }

    template<class... Args>
    void emplace(Args&&... args) {
        c.emplace_back(std::forward<Args>(args)...);
        scratch::push_heap(c.begin(), c.end(), comp);
    }

    void pop() {
        scratch::pop_heap(c.begin(), c.end(), comp);
        c.pop_back();
    }

    // replace_top is NOT a standard library method!
    void replace_top(const value_type& value) {
        scratch::pop_heap(c.begin(), c.end(), comp);
        c.back() = value;
        scratch::push_heap(c.begin(), c.end(), comp);
    }

    // replace_top is NOT a standard library method!
    void replace_top(value_type&& value) {
        scratch::pop_heap(c.begin(), c.end(), comp);
        c.back() = std::move(value);
        scratch::push_heap(c.begin(), c.end(), comp);
    }
};

template<class T, class A, class B>
auto swap(priority_queue<T, A, B>& lhs, priority_queue<T, A, B>& rhs) noexcept
    -> enable_if_t<noexcept(lhs.swap(rhs)), void>
{
    lhs.swap(rhs);
}

} // namespace scratch
