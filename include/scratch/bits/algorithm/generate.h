#pragma once

namespace scratch {

template<class OutIt, class Generator>
void generate(OutIt first, OutIt last, Generator f)
{
    while (first != last) {
        *first = f();
        ++first;
    }
}

template<class OutIt, class Size, class Generator>
OutIt generate_n(OutIt dest, Size count, Generator f)
{
    while (count) {
        *dest = f();
        ++dest;
        --count;
    }
    return dest;
}

} // namespace scratch
