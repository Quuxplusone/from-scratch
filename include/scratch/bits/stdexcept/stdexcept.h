#pragma once

#include "scratch/bits/stdexcept/exception.h"

namespace scratch {

class logic_error : public detail::reasoned_exception {
    using reasoned_exception::reasoned_exception;
};

class domain_error     : public logic_error { using logic_error::logic_error; };
class invalid_argument : public logic_error { using logic_error::logic_error; };
class length_error     : public logic_error { using logic_error::logic_error; };
class out_of_range     : public logic_error { using logic_error::logic_error; };

class runtime_error : public detail::reasoned_exception {
    using reasoned_exception::reasoned_exception;
};

class range_error      : public runtime_error { using runtime_error::runtime_error; };
class overflow_error   : public runtime_error { using runtime_error::runtime_error; };
class underflow_error  : public runtime_error { using runtime_error::runtime_error; };

} // namespace scratch
