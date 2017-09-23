
template<class It, class F>
It find_if(It first, It last, const F& predicate)
{
    while (first != last) {
        if (predicate(*first)) {
            return first;
        }
        ++first;
    }
    return last;
}

template<class It, class T>
It find(It first, It last, const T& value)
{
    while (first != last) {
        if (*first == value) {
            return first;
        }
        ++first;
    }
    return last;
}
