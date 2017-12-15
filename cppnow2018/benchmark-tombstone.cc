#include <benchmark/benchmark.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>
#include "tombstone-traits.h"
#include "optional.h"
#include "robin-hood-set.h"

#if ALSO_TEST_BOOST
 #include "boost/version.hpp"
 #if BOOST_VERSION >= 105600
  #include "boost/optional.hpp"
  namespace boost2 {
    template<class T>
    struct optional : boost::optional<T> {
        bool has_value() const { return bool(*this); }
    };
  } // namespace boost2
  #define boost boost2
 #else // pre-1.56
  #define private protected
   #include "boost/optional.hpp"
  #undef private
  namespace boost2 {
    template<class T>
    struct optional : boost::optional<T> {
        using boost::optional<T>::optional;
        using boost::optional<T>::operator=;
        bool has_value() const { return bool(*this); }
        template<class... As>
        void emplace(As&&... as) {
            this->destroy();
            new (this->m_storage.address()) T(std::forward<As>(as)...);
            this->m_initialized = true;
        }
    };
  } // namespace boost2
  #define boost boost2
 #endif // pre-1.56
#endif // ALSO_TEST_BOOST

#if __cplusplus >= 201703L
#include <optional>
#else // pre-C++17
#include <experimental/optional>
namespace std {
    template<class T>
    struct optional : std::experimental::optional<T> {
        using std::experimental::optional<T>::optional;
        using std::experimental::optional<T>::operator=;
        bool has_value() const { return bool(*this); }
        void reset() { *this = std::experimental::nullopt; }
    };
} // namespace std
#endif // pre-C++17

inline unsigned bernstein_hash(const char *s)
{
    unsigned hashv = 0;
    while (*s) hashv = 33*hashv + *s++;
    return hashv;
}

template<int Spares>
struct Tombstoneable {
    char *s;
    explicit Tombstoneable(const char *t) : s(strdup(t)) {}
    Tombstoneable(Tombstoneable&& rhs) noexcept : s(rhs.s) { rhs.s = nullptr; }
    Tombstoneable& operator=(Tombstoneable&& rhs) noexcept { free(s); s = rhs.s; rhs.s = nullptr; return *this; }
    ~Tombstoneable() { free(s); }
    bool operator==(const Tombstoneable& rhs) const noexcept { return strcmp(s, rhs.s) == 0; }
    unsigned hash() const noexcept { return bernstein_hash(s); }
};

template<int Spares> struct std::hash<Tombstoneable<Spares>> {
    size_t operator()(const Tombstoneable<Spares>& t) const { return t.hash(); }
};

template<int Spares> struct scratch::tombstone_traits<Tombstoneable<Spares>> {
    static constexpr size_t spare_representations = Spares;
    static char special_values[Spares];
    static constexpr void set_spare_representation(Tombstoneable<Spares> *t, size_t idx) {
        t->s = &special_values[idx];
    }
    static constexpr size_t index(const Tombstoneable<Spares> *t) {
        for (int i=0; i < Spares; ++i) {
            if (t->s == &special_values[i]) return i;
        }
        return size_t(-1);
    }
};
template<int Spares> char scratch::tombstone_traits<Tombstoneable<Spares>>::special_values[Spares];

template<class T>
T random_element() {
    char buffer[10];
    buffer[0] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[1] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[2] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[3] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[4] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[5] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[6] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[7] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[8] = '\0';
    return T(buffer);
}

template<template<class> class Optional, class T>
void test_robinhood(benchmark::State& state)
{
    int M = state.range(0);
    scratch::robin_hood_set<Optional, T> s;
    while (state.KeepRunning()) {
        s.clear();
        int erased_count = 0;
        int found_count = 0;
        for (int i=0; i < M/2; ++i) {
            s.insert(random_element<T>());
        }
        for (int i=0; i < M/2; ++i) {
            erased_count += s.erase(random_element<T>());
        }
        for (int i=0; i < M/2; ++i) {
            auto it = s.find(random_element<T>());
            found_count += (it != s.end());
        }
        benchmark::DoNotOptimize(erased_count);
        benchmark::DoNotOptimize(found_count);
    }
}

static int print = []() {
#define PRINT_SIZE(...) \
    printf("sizeof " #__VA_ARGS__ " = %zu\n", sizeof(__VA_ARGS__))
    PRINT_SIZE(scratch::robin_hood_element<std::optional, Tombstoneable<2>>);
#if ALSO_TEST_BOOST
    PRINT_SIZE(scratch::robin_hood_element<boost::optional, Tombstoneable<2>>);
#endif
    PRINT_SIZE(scratch::robin_hood_element<scratch::optional, Tombstoneable<0>>);
    PRINT_SIZE(scratch::robin_hood_element<scratch::optional, Tombstoneable<1>>);
    PRINT_SIZE(scratch::robin_hood_element<scratch::optional, Tombstoneable<2>>);
    return 0;
#undef PRINT_SIZE
}();

void std_optional(benchmark::State& state)
{
    test_robinhood<std::optional, Tombstoneable<2>>(state);
}

#if ALSO_TEST_BOOST
void boost_optional(benchmark::State& state)
{
    test_robinhood<boost::optional, Tombstoneable<2>>(state);
}
#endif

void scratch_tombstone0(benchmark::State& state)
{
    test_robinhood<scratch::optional, Tombstoneable<0>>(state);
}

void scratch_tombstone1(benchmark::State& state)
{
    test_robinhood<scratch::optional, Tombstoneable<1>>(state);
}

void scratch_tombstone2(benchmark::State& state)
{
    test_robinhood<scratch::optional, Tombstoneable<2>>(state);
}

BENCHMARK(std_optional)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
#if ALSO_TEST_BOOST
BENCHMARK(boost_optional)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
#endif
BENCHMARK(scratch_tombstone0)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK(scratch_tombstone1)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK(scratch_tombstone2)->Arg(100)->Arg(1000)->Arg(10'000)->Arg(100'000);
BENCHMARK_MAIN();
