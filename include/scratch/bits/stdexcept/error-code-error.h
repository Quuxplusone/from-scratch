#pragma once

#include "scratch/bits/containers/string.h"
#include "scratch/bits/error-code/error-code.h"

namespace scratch::detail {

template<class Base>
class error_code_error : public Base {
    error_code m_code;
    string m_what;
public:
    explicit error_code_error(error_code v) : m_code(v), m_what(v.to_string()) {}
    explicit error_code_error(error_code v, const string& prefix) : m_code(v), m_what(prefix + ": " + v.to_string()) {}
    explicit error_code_error(error_code v, const char *prefix) : m_code(v), m_what(prefix + (": " + v.to_string())) {}
    explicit error_code_error(int v, const error_category& cat) : m_code(v, cat), m_what(m_code.to_string()) {}
    explicit error_code_error(int v, const error_category& cat, const char *prefix) : m_code(v, cat), m_what(prefix + (": " + m_code.to_string())) {}
    explicit error_code_error(int v, const error_category& cat, string prefix) : m_code(v, cat), m_what(prefix + ": " + m_code.to_string()) {}

    const error_code& code() const noexcept { return m_code; }
    const char* what() const noexcept override { return m_what.c_str(); }
};

} // namespace scratch::detail
