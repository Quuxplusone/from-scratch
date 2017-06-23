#pragma once

#include "scratch/bits/containers/string.h"
#include "scratch/bits/error-code/error-code.h"
#include "scratch/bits/type-traits/integral-constant.h"

namespace scratch::detail {

struct future_category : error_category {
    const char *name() const noexcept override { return "future"; }

    string message(int value) const override {
        switch (value) {
            case 0: return "success";
            case 1: return "broken promise";
            case 2: return "future already retrieved";
            case 3: return "promise already satisfied";
            case 4: return "no state";
        }
        __builtin_unreachable();
    }
};

} // namespace scratch::detail

namespace scratch {

const error_category& future_category() noexcept
{
    static detail::future_category instance;
    return instance;
}

enum class future_errc : signed char {
    success                    = 0,  // non-standard
    broken_promise             = 1,
    future_already_retrieved   = 2,
    promise_already_satisfied  = 3,
    no_state                   = 4,
};

template<> struct is_error_code_enum<future_errc> : true_type {};

error_code make_error_code(future_errc e) { return error_code(static_cast<int>(e), future_category()); }

} // namespace scratch
