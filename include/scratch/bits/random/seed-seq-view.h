#pragma once

#include "scratch/bits/algorithm/min.h"
#include "scratch/bits/algorithm/transform.h"
#include "scratch/bits/containers/vector-view.h"
#include "scratch/bits/type-traits/is-fooible.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>

namespace scratch {

template<class ET>
class seed_seq_view
{
    static_assert(is_constructible_v<uint32_t, const ET&>);
public:
    using result_type = uint32_t;

    seed_seq_view() = default;
    seed_seq_view(seed_seq_view&&) = delete;
    seed_seq_view(const seed_seq_view&) = delete;
    seed_seq_view& operator=(seed_seq_view&&) = delete;
    seed_seq_view& operator=(const seed_seq_view&) = delete;

    seed_seq_view(std::initializer_list<ET> il) : seed_seq_view(il.begin(), il.end()) {}

    template<class It>
    seed_seq_view(It first, It last) : m_data(first, last) {}

    size_t size() const { return m_data.size(); }

    template<class OutIt>
    void param(OutIt dest) const {
        transform(m_data.begin(), m_data.end(), dest, [](auto&& x) { return uint32_t(x); });
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
                (k <= s) ? (k % n) + uint32_t(m_data[k - 1]) :
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
    vector_view<const ET> m_data;
};

} // namespace scratch
