#pragma once

#include "scratch/bits/random/is-seed-generator.h"
#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/false-v.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-signed.h"

#include <cstdint>
#include <utility>

namespace scratch {

template<class T, T a, T c, T m>
class linear_congruential_engine
{
    // If m==0, then we don't mod by m; we just allow T to wrap around.
    static_assert(m == 0 || a < m);
    static_assert(m == 0 || c < m);
public:
    using result_type = T;

    static constexpr T multiplier = a;
    static constexpr T increment = c;
    static constexpr T modulus = m;

    static constexpr result_type default_seed = 1;

    result_type operator()() {
        m_state *= a;
        m_state += c;
        if constexpr (m != 0) {
            m_state %= m;
        }
        return m_state;
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

    static constexpr result_type min() { return (c == 0) ? 1 : 0; }
    static constexpr result_type max() { return m - 1; }

    void seed(result_type s = 1) {
        m_state = (m == 0) ? s : (s % m);
        if constexpr (c == 0) {
            if (m_state == 0) {
                m_state = 1;
            }
        }
    }

    template<class G, class = enable_if_t<is_seed_generator_v<G&&>>>
    void seed(G&& gen) {
        // Generate (lg m) + 3 words of seed data, then seed the PRNG
        // with s[3:] interpreted as a little-endian number.
        if constexpr (uint32_t(m) == m) {
            result_type s[4];
            std::forward<G>(gen).generate(s, s + 4);
            this->seed(s[3]);
        } else if constexpr (uint64_t(m) == m) {
            result_type s[5];
            std::forward<G>(gen).generate(s, s + 5);
            this->seed(s[4] * (uint64_t(1) << 32) + s[3]);
        } else {
            static_assert(false_v<G>, "unimplemented");
        }
    }

    linear_congruential_engine() : linear_congruential_engine(1) {}
    linear_congruential_engine(linear_congruential_engine&&) = default;
    linear_congruential_engine(const linear_congruential_engine&) = default;
    linear_congruential_engine& operator=(linear_congruential_engine&&) = default;
    linear_congruential_engine& operator=(const linear_congruential_engine&) = default;

    explicit linear_congruential_engine(T s) { seed(s); }

    template<class G, class = enable_if_t<is_seed_generator_v<G&&> && !is_same_v<decay_t<G>, linear_congruential_engine>>>
    explicit linear_congruential_engine(G&& gen) {
        this->seed(std::forward<G>(gen));
    }

private:
    T m_state;
};

using minstd_rand0 = linear_congruential_engine<uint32_t, 16807, 0, 2147483647>;
using minstd_rand = linear_congruential_engine<uint32_t, 48271, 0, 2147483647>;

} // namespace scratch
