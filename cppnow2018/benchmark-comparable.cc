#include <benchmark/benchmark.h>
#include <cassert>
#include <type_traits>
#include <vector>
#include "is-relocatable.h"
#include "vector.h"

struct TriviallyComparable {
    TriviallyComparable(int i) : i(i) {}
    int i;
    bool operator==(const TriviallyComparable& rhs) const noexcept { return i == rhs.i; }
    bool operator<(const TriviallyComparable& rhs) const noexcept { return i < rhs.i; }
};

struct NonTriviallyComparable {
    NonTriviallyComparable(int i) : i(i) {}
    int i;
    bool operator==(const NonTriviallyComparable& rhs) const noexcept { return i == rhs.i; }
    bool operator<(const NonTriviallyComparable& rhs) const noexcept { return i < rhs.i; }
};

template<> struct scratch::is_trivially_equality_comparable<TriviallyComparable> : std::true_type {};
template<> struct scratch::is_trivially_less_than_comparable<TriviallyComparable> : std::true_type {};

template<class VectorT>
VectorT new_random_vector(int n)
{
    VectorT result;
    result.reserve(n);
    for (int i=0; i < n - 1; ++i) {
        result.emplace_back(0);
    }
    result.emplace_back(rand() % 2);
    assert(result.size() == result.capacity());
    assert(result.size() == n);
    return result;
}

template<class VectorT, class Comparator>
void test_comparison(benchmark::State& state)
{
    int M = state.range(0);
    VectorT v1, v2;
    static bool b;
    v1 = new_random_vector<VectorT>(M);
    v2 = new_random_vector<VectorT>(M);
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(v1);
        benchmark::DoNotOptimize(v2);
        state.ResumeTiming();
        b = Comparator{}(v1, v2);
        state.PauseTiming();
        benchmark::DoNotOptimize(b);
        v1 = std::move(v2);
        v2 = new_random_vector<VectorT>(M);
    }
}

void std_vector_eq(benchmark::State& state) {
    test_comparison<std::vector<TriviallyComparable>, std::equal_to<>>(state);
}

void scratch_yes_memcmpable_eq(benchmark::State& state) {
    test_comparison<scratch::vector<TriviallyComparable>, std::equal_to<>>(state);
}

void scratch_not_memcmpable_eq(benchmark::State& state) {
    test_comparison<scratch::vector<NonTriviallyComparable>, std::equal_to<>>(state);
}

void std_vector_lt(benchmark::State& state) {
    test_comparison<std::vector<TriviallyComparable>, std::less<>>(state);
}

void scratch_yes_memcmpable_lt(benchmark::State& state) {
    test_comparison<scratch::vector<TriviallyComparable>, std::less<>>(state);
}

void scratch_not_memcmpable_lt(benchmark::State& state) {
    test_comparison<scratch::vector<NonTriviallyComparable>, std::less<>>(state);
}


BENCHMARK(scratch_yes_memcmpable_lt)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK(scratch_not_memcmpable_lt)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK(std_vector_lt)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK(scratch_yes_memcmpable_eq)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK(scratch_not_memcmpable_eq)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK(std_vector_eq)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK_MAIN();
