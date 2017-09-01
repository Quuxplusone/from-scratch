#pragma once

#include "scratch/bits/random/is-seed-generator.h"
#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-signed.h"

#include <cstdint>
#include <utility>

namespace scratch {

// ISAAC (by Bob Jenkins) is a cryptographically secure PRNG.
// This code is a direct translation of http://burtleburtle.net/bob/c/readable.c
// `isaac_engine` is NOT a standard class template!

class isaac_engine {
public:
    using result_type = uint32_t;

    result_type operator()() {
        if (m_idx == 0) {
            isaac_next();
            m_idx = 256;
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
        std::forward<G>(gen).generate(m_mm, m_mm + 256);
    }

    isaac_engine() = default;
    isaac_engine(isaac_engine&&) = default;
    isaac_engine(const isaac_engine&) = default;
    isaac_engine& operator=(isaac_engine&&) = default;
    isaac_engine& operator=(const isaac_engine&) = default;

    template<class G, class = enable_if_t<is_seed_generator_v<G&&> && !is_same_v<decay_t<G>, isaac_engine>>>
    explicit isaac_engine(G&& gen) {
        this->seed(std::forward<G>(gen));
    }

private:
    void isaac_next() {
        ++m_cc;        // cc just gets incremented once per 256 results
        m_bb += m_cc;  // then combined with bb

        for (int i = 0; i < 256; ++i) {
            switch (i % 4) {
                case 0: m_aa ^= (m_aa << 13); break;
                case 1: m_aa ^= (m_aa >> 6); break;
                case 2: m_aa ^= (m_aa << 2); break;
                case 3: m_aa ^= (m_aa >> 16); break;
            }
            uint32_t x = m_mm[i];
            m_aa += m_mm[(i+128) % 256];
            uint32_t y = m_mm[(x>>2) % 256] + m_aa + m_bb;
            m_mm[i] = y;
            m_bb = m_mm[(y>>10) % 256] + x;
            m_output[i] = m_bb;

            // Note that bits 2..9 are chosen from x but 10..17 are chosen
            // from y.  The only important thing here is that 2..9 and 10..17
            // don't overlap.  2..9 and 10..17 were then chosen for speed in
            // the optimized version (rand.c)
            // See http://burtleburtle.net/bob/rand/isaac.html
            // for further explanations and analysis.
        }
    }

    int m_idx = 0;
    uint32_t m_output[256];
    uint32_t m_mm[256] = {};
    uint32_t m_aa = 0;
    uint32_t m_bb = 0;
    uint32_t m_cc = 0;
};

} // namespace scratch
