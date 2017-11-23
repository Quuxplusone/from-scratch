#pragma once

namespace scratch {

template<class It, class OutIt, class UnaryOp>
OutIt transform(It first, It last, OutIt dest, UnaryOp f)
{
    while (first != last) {
        *dest = f(*first);
        ++dest;
        ++first;
    }
    return dest;
}

template<class It, class Size, class OutIt, class UnaryOp>
OutIt transform_n(It first, Size count, OutIt dest, UnaryOp f)
{
    while (count) {
        *dest = f(*first);
        ++dest;
        ++first;
        --count;
    }
    return dest;
}

} // namespace scratch
