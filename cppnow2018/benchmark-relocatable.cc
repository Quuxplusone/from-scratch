#include <benchmark/benchmark.h>
#include <cassert>
#include <memory>
#include <type_traits>
#include <vector>

struct R : std::unique_ptr<int> {
    using is_trivially_relocatable = std::true_type;
};

struct NR : std::unique_ptr<int> {};

#ifdef _LIBCPP_SUPPORTS_TRIVIALLY_RELOCATABLE
static_assert(std::is_relocatable<R>::value, "");
static_assert(std::is_relocatable<NR>::value, "");
static_assert(std::is_trivially_relocatable<R>::value, "");
static_assert(!std::is_trivially_relocatable<NR>::value, "");
#endif

template<class VectorT>
void test_reserve(benchmark::State& state)
{
    int M = state.range(0);
    for (auto _ : state) {
        VectorT v(M);
        assert(v.capacity() == M);
        state.ResumeTiming();
        v.reserve(M+1);
        state.PauseTiming();
        benchmark::DoNotOptimize(v);
    }
}

BENCHMARK_TEMPLATE(test_reserve, std::vector<R>)->Arg(10'000);
BENCHMARK_TEMPLATE(test_reserve, std::vector<NR>)->Arg(10'000);
BENCHMARK_MAIN();
