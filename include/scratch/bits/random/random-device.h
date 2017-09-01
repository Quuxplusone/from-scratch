#pragma once

#include "scratch/bits/random/is-seed-generator.h"
#include "scratch/bits/stdexcept/stdexcept.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-signed.h"

#include <cstdint>
#include <fcntl.h>
#include <unistd.h>

namespace scratch {

class random_device {
public:
    random_device() {
        m_fd = ::open("/dev/urandom", O_RDONLY);
        if (m_fd < 0) {
            throw runtime_error(exception::nocopy, "couldn't open /dev/urandom");
        }
    }
    random_device(random_device&&) = delete;
    random_device(const random_device&) = delete;
    random_device& operator=(random_device&&) = delete;
    random_device& operator=(const random_device&) = delete;
    ~random_device() {
        ::close(m_fd);
    }

    using result_type = uint32_t;

    result_type operator()() {
        uint32_t r;
        this->generate(&r, &r+1);
        return r;
    }

    template<class UInt>
    void generate(UInt *first, UInt *last) {
        static_assert(is_integral_v<UInt> && is_unsigned_v<UInt>);
        int bytes_to_read = (last - first) * sizeof *first;
        int bytes_read = ::read(m_fd, &*first, bytes_to_read);
        if (bytes_read < bytes_to_read) {
            throw runtime_error(exception::nocopy, "failed to read N bytes from /dev/urandom");
        }
    }

    void discard(unsigned long long) {}

    static constexpr result_type min() { return result_type(0); }
    static constexpr result_type max() { return result_type(-1); }

    void seed(result_type) {}

    template<class G, class = enable_if_t<is_seed_generator_v<G&&>>>
    void seed(G&&) {}

private:
    int m_fd;
};

} // namespace scratch
