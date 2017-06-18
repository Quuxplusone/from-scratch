#include <type_traits>
#include <benchmark/benchmark.h>

template<class T> static T *instance() { static T t; return &t; }

template<class F, class T, class = void> struct can_dynamic_cast_impl : std::false_type {};
template<class F, class T> struct can_dynamic_cast_impl<F, T, decltype(void(dynamic_cast<T>((F)nullptr)))> : std::true_type {};
template<class F, class T> struct can_dynamic_cast : can_dynamic_cast_impl<F, T, void> {};

template<class F, class T> constexpr bool can_dynamic_cast_v = can_dynamic_cast<F, T>::value;

template<bool B> using bool_if_t = std::enable_if_t<B, bool>;


template<class To, class From, bool_if_t<can_dynamic_cast_v<From*, To*>> = true>
void run_benchmark(std::true_type, From *f) {
    To *p = dynamic_cast<To*>(f);
    benchmark::DoNotOptimize(p);
}

template<class To, class From, bool_if_t<!can_dynamic_cast_v<From*, To*>> = true>
void run_benchmark(std::true_type, From *) {}

template<class To, class From, bool_if_t<can_dynamic_cast_v<From*, To*>> = true>
void run_benchmark(std::false_type, From *f) {
    To *p = dynamicast<To*>(f);
    benchmark::DoNotOptimize(p);
}

template<class To, class From, bool_if_t<!can_dynamic_cast_v<From*, To*>> = true>
void run_benchmark(std::false_type, From *) {}

#define TEST(ClassX) \
static void BM_native_##ClassX(benchmark::State& state) { \
  while (state.KeepRunning()) { \
    benchmark_to<ClassX>(std::true_type{}); \
  } \
} \
static void BM_dynamicast_##ClassX(benchmark::State& state) { \
  while (state.KeepRunning()) { \
    benchmark_to<ClassX>(std::false_type{}); \
  } \
} \
BENCHMARK(BM_native_##ClassX); \
BENCHMARK(BM_dynamicast_##ClassX);
