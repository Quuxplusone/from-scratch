#pragma once

#include "scratch/bits/stdexcept/error-code-error.h"
#include "scratch/bits/stdexcept/stdexcept.h"

namespace scratch {

class system_error : public detail::error_code_error<runtime_error> {
    using detail::error_code_error<runtime_error>::error_code_error;
};

} // namespace scratch
