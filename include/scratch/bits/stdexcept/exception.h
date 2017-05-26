#pragma once

namespace scratch {

class exception
{
public:
    exception() noexcept = default;
    exception(const exception&) noexcept = default;
    exception& operator= (const exception&) noexcept = default;

    virtual const char *what() const noexcept { return ""; }
    virtual ~exception() {}

    struct nocopy_t {};
    static constexpr nocopy_t nocopy = {};
};

} // namespace scratch

namespace scratch::detail {

// This is the base class for logic_error and runtime_error,
// both of which have the same need for a "what" string.
// TODO: this class does not own its string. It should
// have a way to store a shared_ptr to an allocated string.
// For now, construct with reasoned_exception(exception::nocopy, "foo").

class reasoned_exception : public scratch::exception
{
public:
    reasoned_exception() noexcept : m_what("") {}
    explicit reasoned_exception(exception::nocopy_t, const char *what) noexcept : m_what(what) {}

    const char *what() const noexcept override { return m_what; }
private:
    const char *m_what;
};

} // namespace scratch::detail
