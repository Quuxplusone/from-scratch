#include "scratch/memory_resource"
#include "scratch/vector"
#include "scratch/bits/containers/propagating-polymorphic-allocator.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <vector>

using Heap = scratch::pmr::monotonic_buffer_resource;
using AllocR = scratch::pmr::propagating_polymorphic_allocator<int>;
using AllocNR = scratch::pmr::polymorphic_allocator<int>;

#ifdef __clang__
using VectorR = std::vector<int, AllocR>;
using VectorNR = std::vector<int, AllocNR>;
#else
using VectorR = scratch::vector<int, AllocR>;
using VectorNR = scratch::vector<int, AllocNR>;
#endif

struct Widget {
    char buffer[1000];
    Heap h{buffer, sizeof buffer};
    VectorNR v;

    Widget() : v(AllocNR(&h)) {}
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    ~Widget() { memset(buffer, 0xFF, sizeof buffer); }

    void add(int i) {
        v.push_back(i);
    }

    void replace_all(Widget& rhs) {
        v = std::move(rhs.v);
    }
};

int main()
{
    // make sure nobody allocates memory without saying how
    scratch::pmr::set_default_resource(scratch::pmr::null_memory_resource());

    auto w1 = std::make_unique<Widget>();
    for (int i=0; i < 10; ++i) {
        w1->add(i);
    }

    auto w2 = std::make_unique<Widget>();
    w2->replace_all(*w1);  // move w1's v into w2

    assert(w1->v.size() == 10);  // assert it didn't actually move

    try {
        auto v2 = w1->v;  // it's illegal (at runtime) to copy v without saying where the RAM comes from
        assert(false);
    } catch (const std::bad_alloc&) {
        puts("awesome");
    }

    VectorNR v(w1->v, scratch::pmr::new_delete_resource());

    w1 = nullptr;  // w1's heap is now gone

    puts("local v:");
    for (int i : v) {
        printf("%d\n", i);  // v is still around
    }

    puts("w2->v:");
    for (int i : w2->v) {
        printf("%d\n", i);  // w2's v is also still around
    }
}
