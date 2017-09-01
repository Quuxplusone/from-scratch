#pragma once

#include "scratch/bits/random/is-seed-generator.h"
#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-signed.h"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace scratch {

// ISAAC (by Bob Jenkins) is a cryptographically secure PRNG.
// This code is a direct translation of http://burtleburtle.net/bob/c/rand.c
// `isaac_fast_engine` is NOT a standard class template!

template<size_t LogN>
class isaac_fast_engine {
public:
    using result_type = uint32_t;
    static constexpr size_t randsizl = LogN;
    static constexpr size_t randsiz = size_t(1) << LogN;

    result_type operator()() {
        if (m_idx == 0) {
            isaac_next();
            m_idx = randsiz;
        }
        return m_output[--m_idx];
    }

    template<class UInt>
    void generate(UInt *first, UInt *last) {
        static_assert(is_integral_v<UInt> && is_unsigned_v<UInt>);
        while (first != last) {
            *first++ = (*this)();
        }
    }

    void discard(unsigned long long z) {
        while (z--) {
            (*this)();
        }
    }

    static constexpr result_type min() { return result_type(0); }
    static constexpr result_type max() { return result_type(-1); }

    template<class G, class = enable_if_t<is_seed_generator_v<G&&>>>
    void seed(G&& gen) {
        std::forward<G>(gen).generate(m_mm, m_mm + randsiz);
    }

    isaac_fast_engine() = default;
    isaac_fast_engine(isaac_fast_engine&&) = default;
    isaac_fast_engine(const isaac_fast_engine&) = default;
    isaac_fast_engine& operator=(isaac_fast_engine&&) = default;
    isaac_fast_engine& operator=(const isaac_fast_engine&) = default;

    template<class G, class = enable_if_t<is_seed_generator_v<G&&> && !is_same_v<decay_t<G>, isaac_fast_engine>>>
    explicit isaac_fast_engine(G&& gen) {
        this->seed(std::forward<G>(gen));
    }

private:
    void isaac_next() {
        uint32_t c = ++m_cc;
        uint32_t b = m_bb + m_cc;
        uint32_t a = m_aa;
        uint32_t *mm = m_mm;
        uint32_t *r = m_output;

        uint32_t *m = mm;
        uint32_t *m2 = mm + (randsiz / 2);
        uint32_t *const mend = m2;

        auto ind = [&](auto x) {
            return mm[(x >> 2) % randsiz];
        };
        auto rngstep = [&](auto mix) {
            uint32_t x = *m;
            a = (a ^ (mix)) + *m2++;
            uint32_t y = *m++ = ind(x) + a + b;
            *r++ = b = ind(y >> randsizl) + x;
        };

        while (m < mend) {
            rngstep(a << 13);
            rngstep(a >> 6);
            rngstep(a << 2);
            rngstep(a >> 16);
        }
        m2 = mm;
        while (m2 < mend) {
            rngstep(a << 13);
            rngstep(a >> 6);
            rngstep(a << 2);
            rngstep(a >> 16);
        }

        m_aa = a;
        m_bb = b;
        m_cc = c;
    }

    int m_idx = 0;
    uint32_t m_output[randsiz];
    uint32_t m_mm[randsiz] = {};
    uint32_t m_aa = 0;
    uint32_t m_bb = 0;
    uint32_t m_cc = 0;
};

using isaac = isaac_fast_engine<8>;

} // namespace scratch
