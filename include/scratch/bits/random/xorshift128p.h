#pragma once

#include "scratch/bits/random/is-seed-generator.h"
#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-foo.h"

#include <cstdint>
#include <utility>

namespace scratch {

class xorshift128p
{
public:
    using result_type = uint64_t;

    static constexpr result_type default_seed = 1;

    // https://stackoverflow.com/a/34432126/1424877
    result_type operator()() {
        uint64_t a = m_state[0];
        uint64_t b = m_state[1];

        m_state[0] = b;
        a ^= a << 23;
        a ^= a >> 18;
        a ^= b;
        a ^= b >> 5;
        m_state[1] = a;

        return a + b;
    }

    template<class UInt>
    void generate(UInt *first, UInt *last) {
        while (first != last) {
            *first++ = (*this)();
        }
    }

    void discard(unsigned long long z) {
        while (z--) {
            (*this)();
        }
    }

    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return uint64_t(-1); }

    void seed(uint64_t s = 1) {
        // https://en.wikipedia.org/w/index.php?title=Xorshift&oldid=910503315#Initialization
	m_state[0] = splitmix64(&s);
	m_state[1] = splitmix64(&s);
    }

    template<class G, class = enable_if_t<is_seed_generator_v<G&&>>>
    void seed(G&& gen) {
        uint32_t s32[4];
        std::forward<G>(gen).generate(s32, s32 + 4);
        uint64_t s64[2] = {
            (uint64_t(s32[0]) << 32) | uint64_t(s32[1]),
            (uint64_t(s32[2]) << 32) | uint64_t(s32[3]),
        };
        m_state[0] = splitmix64(&s64[0]);
        m_state[1] = splitmix64(&s64[1]);
    }

    xorshift128p() : xorshift128p(1) {}
    xorshift128p(xorshift128p&&) = default;
    xorshift128p(const xorshift128p&) = default;
    xorshift128p& operator=(xorshift128p&&) = default;
    xorshift128p& operator=(const xorshift128p&) = default;

    explicit xorshift128p(uint64_t s) { seed(s); }

    template<class G, class = enable_if_t<is_seed_generator_v<G&&> && !is_same_v<decay_t<G>, xorshift128p>>>
    explicit xorshift128p(G&& gen) {
        this->seed(std::forward<G>(gen));
    }

private:
    static uint64_t splitmix64(uint64_t *state) {
        // https://en.wikipedia.org/w/index.php?title=Xorshift&oldid=910503315#Initialization
        uint64_t result = *state;
        *state = result + 0x9E3779B97F4A7C15uLL;
        result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9uLL;
        result = (result ^ (result >> 27)) * 0x94D049BB133111EBuLL;
        return result ^ (result >> 31);
    }

    uint64_t m_state[2];
};

} // namespace scratch
