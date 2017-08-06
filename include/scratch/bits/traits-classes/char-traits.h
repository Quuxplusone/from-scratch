#pragma once

namespace scratch {

template<class> struct string_related_char_traits;
template<class> struct iostream_related_char_traits;

template<>
struct string_related_char_traits<char> {
};

template<>
struct iostream_related_char_traits<char> {
};

template<class T>
struct char_traits<T> : string_related_char_traits<T>, iostream_related_char_traits<T> {
    using char_type = T;
};

} // namespace scratch
