#include <benchmark/benchmark.h>
#include <cstring>
#include <type_traits>
#include <utility>
#include <optional>
#include <vector>
#include "robin-hood-set.h"
#include "ska-flathash.h"


inline unsigned bernstein_hash(const char *s)
{
    unsigned hashv = 0;
    while (*s) hashv = 33*hashv + *s++;
    return hashv;
}

template<int Spares>
struct Tombstoneable {
    const char *s;
    explicit Tombstoneable(const char *t) : s(t) {}
    bool operator==(const Tombstoneable& rhs) const noexcept { return strcmp(s, rhs.s) == 0; }
    unsigned hash() const noexcept { return bernstein_hash(s); }
};

template<int Spares> struct std::hash<Tombstoneable<Spares>> {
    size_t operator()(const Tombstoneable<Spares>& t) const { return t.hash(); }
};

#ifdef _LIBCPP_SUPPORTS_TOMBSTONE_TRAITS
template<int Spares> struct std::tombstone_traits<Tombstoneable<Spares>> {
    static char special_values[Spares];
    static constexpr size_t spare_representations = Spares;
    static constexpr void set_spare_representation(Tombstoneable<Spares> *p, size_t i) {
        *(char**)p = &special_values[i];
    }
    static constexpr size_t index(const Tombstoneable<Spares> *p) {
        for (int i=0; i < Spares; ++i) {
            if (*(char**)p == &special_values[i])
                return i;
        }
        return size_t(-1);
    }
};
template<int Spares> char std::tombstone_traits<Tombstoneable<Spares>>::special_values[Spares];
#endif

const char *seven_letter_string() {
    char *buffer = new char[8];
    buffer[0] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[1] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[2] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[3] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[4] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[5] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[6] = "abcdefghijklmnopqrstuvwxyz"[rand() % 16];
    buffer[7] = '\0';
    return buffer;
}


static size_t cur_ = 0;
static size_t hwm_ = 0;

template<class T>
struct MyAllocator {
    using value_type = T;
    template<class U> struct rebind { using other = MyAllocator<U>; };
    MyAllocator() = default;
    template<class U> MyAllocator(const MyAllocator<U>&) {}
    template<class U> MyAllocator& operator=(const MyAllocator<U>&) { return *this; }
    constexpr bool operator==(MyAllocator&) const noexcept { return true; }
    constexpr bool operator!=(MyAllocator&) const noexcept { return false; }
    T *allocate(size_t n) const {
        cur_ += n * sizeof(T);
        if (cur_ > hwm_) hwm_ = cur_;
        return std::allocator<T>().allocate(n);
    }
    void deallocate(T *p, size_t n) const {
        cur_ -= n * sizeof(T);
        std::allocator<T>().deallocate(p, n);
    }
};

template<class SetT, class T = typename SetT::value_type>
void test_set(benchmark::State& state)
{
    int M = state.range(0);
    std::vector<T> to_insert, to_erase, to_find;
    for (int i=0; i < M; ++i) {
        to_insert.emplace_back(seven_letter_string());
        to_erase.emplace_back(seven_letter_string());
        to_find.emplace_back(seven_letter_string());
    }
    __int128 total_hwms = 0;
    size_t count_hwms = 0;
    for (auto _ : state) {
        SetT s;
        hwm_ = cur_;
        int erased_count = 0;
        int found_count = 0;
        for (const auto& r : to_insert) {
            s.insert(r);
        }
        for (const auto& r : to_erase) {
            erased_count += s.erase(r);
        }
        for (const auto& r : to_find) {
            found_count += (s.find(r) != s.end());
        }
        benchmark::DoNotOptimize(erased_count);
        benchmark::DoNotOptimize(found_count);
        total_hwms += hwm_;
        count_hwms += 1;
    }
    state.counters["HWM"] = total_hwms / count_hwms;
}

static int print = []() {
#define PRINT_SIZE(...) \
    printf("sizeof " #__VA_ARGS__ " = %zu\n", sizeof(__VA_ARGS__))
    PRINT_SIZE(scratch::robin_hood_element<std::optional, Tombstoneable<0>>);
    PRINT_SIZE(scratch::robin_hood_element<std::optional, Tombstoneable<1>>);
    PRINT_SIZE(scratch::robin_hood_element<std::optional, Tombstoneable<2>>);
    return 0;
#undef PRINT_SIZE
}();

template<class T> using ska_set =
    ska::flat_hash_set<T, std::hash<T>, std::equal_to<T>, MyAllocator<T>>;
template<template<class> class O, class T> using rh_set =
    scratch::robin_hood_set<O, T, std::hash<T>, std::equal_to<T>, MyAllocator<T>>;

BENCHMARK_TEMPLATE(test_set, ska_set<Tombstoneable<2>>)->Arg(1000);
BENCHMARK_TEMPLATE(test_set, rh_set<std::optional, Tombstoneable<0>>)->Arg(1000);
BENCHMARK_TEMPLATE(test_set, rh_set<std::optional, Tombstoneable<1>>)->Arg(1000);
BENCHMARK_TEMPLATE(test_set, rh_set<std::optional, Tombstoneable<2>>)->Arg(1000);
BENCHMARK_MAIN();
