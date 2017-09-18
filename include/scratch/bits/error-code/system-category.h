#pragma once

#include "scratch/bits/containers/string.h"
#include "scratch/bits/error-code/error-code.h"

namespace scratch::detail {

struct system_category : error_category {
    const char *name() const noexcept override { return "system"; }

    string message(int value) const override {
        switch (value) {
            case 0: return "success";
        }
        __builtin_unreachable();
    }
};

} // namespace scratch::detail

namespace scratch {

const error_category& system_category() noexcept
{
    static detail::system_category instance;
    return instance;
}

} // namespace scratch
