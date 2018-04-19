#include <benchmark/benchmark.h>
#include <memory>
#include <type_traits>
#include <vector>

struct T : std::unique_ptr<int> {
    using is_trivially_comparable = std::true_type;
};

struct NT : std::unique_ptr<int> {};

#ifdef _LIBCPP_SUPPORTS_TRIVIALLY_COMPARABLE
static_assert(std::is_trivially_comparable<T>::value, "");
static_assert(!std::is_trivially_comparable<NT>::value, "");
#endif

template<class VectorT, class Comparator>
void test_compare(benchmark::State& state)
{
    int M = state.range(0);
    VectorT v1(M), v2(M);
    static bool b;
    for (auto _ : state) {
        b = Comparator{}(v1, v2);
        benchmark::DoNotOptimize(b);
        benchmark::DoNotOptimize(v1);
        benchmark::DoNotOptimize(v2);
    }
}

BENCHMARK_TEMPLATE(test_compare, std::vector<T>, std::equal_to<>)->Arg(10'000);
BENCHMARK_TEMPLATE(test_compare, std::vector<NT>, std::equal_to<>)->Arg(10'000);
BENCHMARK_TEMPLATE(test_compare, std::vector<T>, std::less<>)->Arg(10'000);
BENCHMARK_TEMPLATE(test_compare, std::vector<NT>, std::less<>)->Arg(10'000);

BENCHMARK_MAIN();
