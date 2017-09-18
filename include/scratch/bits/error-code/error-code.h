#pragma once

#include "scratch/bits/containers/string.h"
#include "scratch/bits/functional/less.h"
#include "scratch/bits/type-traits/integral-constant.h"

namespace scratch {

template<class E> struct is_error_code_enum : false_type {};

class error_category {
public:
    constexpr error_category() noexcept = default;
    error_category(const error_category&) = delete;

    virtual const char *name() const noexcept = 0;
    virtual string message(int value) const = 0;
    virtual ~error_category() = default;

    bool operator==(const error_category& rhs) const noexcept { return this == &rhs; }
    bool operator!=(const error_category& rhs) const noexcept { return this != &rhs; }
    bool operator<(const error_category& rhs) const noexcept { return less<>()(this, &rhs); }
    bool operator<=(const error_category& rhs) const noexcept { return less_equal<>()(this, &rhs); }
    bool operator>(const error_category& rhs) const noexcept { return greater<>()(this, &rhs); }
    bool operator>=(const error_category& rhs) const noexcept { return greater_equal<>()(this, &rhs); }
};

const error_category& system_category() noexcept;  // forward declaration

class error_code {
    int m_value;
    const error_category *m_category;
public:
    int value() const noexcept { return m_value; }
    const error_category& category() const noexcept { return *m_category; }

    error_code() noexcept : error_code(0, system_category()) {}
    error_code(int v, const error_category& cat) noexcept : m_value(v), m_category(&cat) {}

    template<class E, class = enable_if_t<is_error_code_enum<E>::value>>
    error_code(E v) {
        *this = make_error_code(v);
    }

    explicit operator bool() const noexcept { return m_value; }

    void clear() noexcept { *this = error_code(0, system_category()); }

    string message() const { return m_category->message(m_value); }
    string to_string() const { return m_category->name() + (":" + message()); }
};

// The standard provides only operators == != <; the other three are non-standard.
inline bool operator==(const error_code& a, const error_code& b) noexcept { return a.category() == b.category() && a.value() == b.value(); }
inline bool operator!=(const error_code& a, const error_code& b) noexcept { return !(a == b); }
inline bool operator< (const error_code& a, const error_code& b) noexcept { return a.category() < b.category() || (a.category() == b.category() && a.value() < b.value()); }
inline bool operator<=(const error_code& a, const error_code& b) noexcept { return !(b < a); }
inline bool operator> (const error_code& a, const error_code& b) noexcept { return (b < a); }
inline bool operator>=(const error_code& a, const error_code& b) noexcept { return !(a < b); }

} // namespace scratch
