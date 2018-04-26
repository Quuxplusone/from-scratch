#pragma once

#include <utility>

namespace scratch::detail {

template<class RandomIt, class Diff, class Compare>
void sift_downward(RandomIt first, Diff current_index, Diff last_index, Compare less)
{
    using std::swap;

    while (current_index < last_index) {
        RandomIt current = first + current_index;
        Diff left_child_index = (2 * current_index) + 1;
        Diff right_child_index = (2 * current_index) + 2;

        if (right_child_index < last_index) {
            RandomIt left_child = first + left_child_index;
            RandomIt right_child = first + right_child_index;
            Diff min_child_index = less(*left_child, *right_child) ? left_child_index : right_child_index;
            RandomIt min_child = first + min_child_index;

            if (less(*min_child, *current)) {
                swap(*current, *min_child);
                current_index = min_child_index;
            } else {
                return;
            }
        } else if (left_child_index < last_index) {
            Diff min_child_index = left_child_index;
            RandomIt min_child = first + left_child_index;
            if (less(*min_child, *current)) {
                swap(*current, *min_child);
                current_index = min_child_index;
            } else {
                return;
            }
        } else {
            return;
        }
    }
}

template<class RandomIt, class Diff, class Compare>
void sift_upward(RandomIt first, Diff current_index, Compare less)
{
    using std::swap;

    while (current_index) {
        RandomIt current = first + current_index;
        Diff parent_index = (current_index - 1) / 2;
        RandomIt parent = first + parent_index;

        if (less(*current, *parent)) {
            swap(*current, *parent);
            current_index = parent_index;
        } else {
            return;
        }
    }
}

} // namespace scratch::detail

namespace scratch {

template<class RandomIt, class Compare>
void make_heap(RandomIt first, RandomIt last, Compare less)
{
    auto last_index = (last - first);
    // We could build the heap "top-down", by sifting each element [0..n) upward.
    // But libc++ observes that it is faster to build it "bottom-up", by assuming that
    // elements [n/2 .. n) are already in plausible spots, and then sifting each element (n/2 .. 0] downward!
    auto current_index = (last_index / 2);
    while (current_index) {
        --current_index;
        detail::sift_downward(first, current_index, last_index, less);
    }
}

template<class RandomIt, class Compare>
void push_heap(RandomIt first, RandomIt last, Compare less)
{
    // TODO
    auto last_index = (last - first);
    --last_index;
    detail::sift_upward(first, last_index, less);
}

template<class RandomIt, class Compare>
void pop_heap(RandomIt first, RandomIt last, Compare less)
{
    auto last_index = (last - first);
    --last_index;
    if (last_index) {
        using std::swap;
        swap(*first, *(first + last_index));
        detail::sift_downward(first, 0, last_index, less);
    }
}

template<class RandomIt, class Compare>
void sort_heap(RandomIt first, RandomIt last, Compare less)
{
    while (first != last) {
        scratch::pop_heap(first, last, less);
        --last;
    }
}

template<class RandomIt>
void make_heap(RandomIt first, RandomIt last)
{
    make_heap(first, last, [](auto&& a, auto&& b){ return a < b; });
}

template<class RandomIt>
void push_heap(RandomIt first, RandomIt last)
{
    push_heap(first, last, [](auto&& a, auto&& b){ return a < b; });
}

template<class RandomIt>
void pop_heap(RandomIt first, RandomIt last)
{
    pop_heap(first, last, [](auto&& a, auto&& b){ return a < b; });
}

template<class RandomIt>`
void sort_heap(RandomIt first, RandomIt last)
{
    sort_heap(first, last, [](auto&& a, auto&& b){ return a < b; });
}

} // namespace scratch
