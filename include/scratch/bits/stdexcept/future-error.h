#pragma once

#include "scratch/bits/stdexcept/error-code-error.h"
#include "scratch/bits/stdexcept/stdexcept.h"

namespace scratch {

class future_error : public detail::error_code_error<logic_error> {
public:
    explicit future_error(future_errc ec) : detail::error_code_error<logic_error>(ec) {}
};

} // namespace scratch
