#pragma once

#include <type_traits>

namespace scratch {

using std::false_type;

template<class T>
struct is_relocatable : false_type {};

} // namespace scratch
