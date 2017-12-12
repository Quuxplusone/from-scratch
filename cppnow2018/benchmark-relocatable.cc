#include <benchmark/benchmark.h>
#include <cassert>
#include <type_traits>
#include <vector>
#include "is-relocatable.h"
#include "vector.h"

struct RelocatableUniquePtr {
    RelocatableUniquePtr(int *p) : p(p) {}
    std::unique_ptr<int> p;
};

struct NonRelocatableUniquePtr {
    NonRelocatableUniquePtr(int *p) : p(p) {}
    std::unique_ptr<int> p;
};

template<> struct scratch::is_relocatable<RelocatableUniquePtr> : std::true_type {};

template<class VectorT>
VectorT new_random_vector(int n)
{
    VectorT result;
    result.reserve(n);
    for (int i=0; i < n; ++i) {
        result.emplace_back(rand() % 2 ? nullptr : new int);
    }
    assert(result.size() == result.capacity());
    assert(result.size() == n);
    return result;
}

template<class VectorT, class T = typename VectorT::value_type>
void test_relocation(benchmark::State& state)
{
    int M = state.range(0);
    VectorT v;
    while (state.KeepRunning()) {
        state.PauseTiming();
        v = new_random_vector<VectorT>(M);
        state.ResumeTiming();
        v.push_back(nullptr);
        benchmark::DoNotOptimize(v);
    }
}

void std_vector(benchmark::State& state)
{
    test_relocation<std::vector<RelocatableUniquePtr>>(state);
}

void scratch_yes_relocatable(benchmark::State& state)
{
    test_relocation<scratch::vector<RelocatableUniquePtr>>(state);
}

void scratch_not_relocatable(benchmark::State& state)
{
    test_relocation<scratch::vector<NonRelocatableUniquePtr>>(state);
}

BENCHMARK(std_vector)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK(scratch_yes_relocatable)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK(scratch_not_relocatable)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK_MAIN();
