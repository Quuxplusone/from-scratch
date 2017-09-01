#pragma once

#include "scratch/bits/algorithm/min.h"
#include "scratch/bits/containers/vector.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>

namespace scratch {

class seed_seq
{
public:
    using result_type = uint32_t;

    seed_seq() = default;
    seed_seq(seed_seq&&) = delete;
    seed_seq(const seed_seq&) = delete;
    seed_seq& operator=(seed_seq&&) = delete;
    seed_seq& operator=(const seed_seq&) = delete;

    template<class T>
    seed_seq(std::initializer_list<T> il) : seed_seq(il.begin(), il.end()) {}

    template<class It>
    seed_seq(It first, It last) : m_data(first, last) {}

    size_t size() const { return m_data.size(); }

    template<class OutIt>
    void param(OutIt dest) const {
        copy(m_data.begin(), m_data.end(), dest);
    }

    template<class UInt>
    void generate(UInt *first, UInt *last) const {
        static_assert(is_integral_v<UInt> && is_unsigned_v<UInt>);
        static_assert(uint32_t(UInt(0x8b8b8b8bu)) == 0x8b8b8b8bu);

        size_t n = last - first;
        size_t s = m_data.size();
        auto T = [](uint32_t x) -> uint32_t { return x ^ (x >> 27); };
        auto B = [&](size_t i) -> uint32_t { return first[i % n]; };
        auto B_ = [&](size_t i) -> auto& { return first[i % n]; };

        for (size_t k = 0; k < n; ++k) {
            B_(k) = 0x8b8b8b8bu;
        }

        uint32_t t = (n >= 623) ? 11 : (n >= 68) ? 7 : (n >= 39) ? 5 : (n >= 7) ? 3 : (n - 1)/2;
        size_t p = (n - t) / 2;
        size_t q = p + t;

        size_t m = max(s + 1, n);
        for (size_t k = 0; k < m; ++k) {
            uint32_t r1 = 1664525u * T(B(k) ^ B(k+p) ^ B(k-1));
            uint32_t r2 = r1 + (
                (k == 0) ? s :
                (k <= s) ? (k % n) + m_data[k - 1] :
                           (k % n)
            );
            B_(k+p) = B(k+p) + r1;
            B_(k+q) = B(k+q) + r2;
            B_(k) = r2;
        }
        for (size_t k = m; k < m + n; ++k) {
            uint32_t r3 = 1566083941 * T(B(k) + B(k+p) + B(k-1));
            uint32_t r4 = r3 - (k % n);
            B_(k+p) = B(k+p) ^ r3;
            B_(k+q) = B(k+q) ^ r4;
            B_(k) = r4;
        }
    }

private:
    vector<uint32_t> m_data;
};

} // namespace scratch
