#pragma once

#include <cstddef>

namespace scratch {

template<class> struct string_related_char_traits;
template<class> struct iostream_related_char_traits;

template<>
struct string_related_char_traits<char>
{
    static constexpr size_t length(const char *s) {
        for (size_t n = 0; true; ++s, ++n) {
            if (*s == '\0') return n;
        }
    }
    static constexpr void assign(char& r, const char& a) {
        r = a;
    }
#if 0
    static void assign(char *p, size_t count, char a) {
        while (count) {
            *p++ = a;
            --count;
        }
    }
#endif
    static void copy(char *dst, const char *src, size_t count) {
        while (count) {
            *dst++ = *src++;
            --count;
        }
    }
};

template<>
struct iostream_related_char_traits<char> {
};

template<class T>
struct char_traits : string_related_char_traits<T>, iostream_related_char_traits<T> {
    using char_type = T;
};

} // namespace scratch
