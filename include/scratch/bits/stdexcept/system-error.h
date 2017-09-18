#pragma once

#include "scratch/bits/containers/string.h"
#include "scratch/bits/error-code/error-code.h"
#include "scratch/bits/stdexcept/stdexcept.h"

namespace scratch {

class system_error : public runtime_error {
    error_code m_code;
    string m_what;
public:
    explicit system_error(error_code v) : m_code(v), m_what(v.to_string()) {}
    explicit system_error(error_code v, const string& prefix) : m_code(v), m_what(prefix + ": " + v.to_string()) {}
    explicit system_error(error_code v, const char *prefix) : m_code(v), m_what(prefix + (": " + v.to_string())) {}
    explicit system_error(int v, const error_category& cat) : m_code(v, cat), m_what(v.to_string()) {}
    explicit system_error(int v, const error_category& cat, const char *prefix) : m_code(v, cat), m_what(prefix + (": " + v.to_string())) {}
    explicit system_error(int v, const error_category& cat, string prefix) : m_code(v, cat), m_what(prefix + ": " + v.to_string()) {}

    const error_code& code() const noexcept { return m_code; }
    const char* what() const noexcept { return m_what.c_str(); }
};

} // namespace scratch
