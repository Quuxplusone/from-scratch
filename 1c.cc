#include <assert.h>
#include <vector>

template<class It>
concept bool Iterator = requires(It it) {
    *it;
    ++it;
    it == it;
    it != it;
};

template<class T>
concept bool Iterable = Iterator<typename T::iterator> && requires(T ctr) {
    { begin(ctr) } -> typename T::iterator;
    { end(ctr) } -> typename T::iterator;
};

template<Iterable Ctr, class T>
auto myfind(Ctr& c, T value) -> typename Ctr::iterator
{
    auto it = begin(c);
    while (it != end(c) && !(*it == value)) {
        ++it;
    }
    return it;
}

int main()
{
    std::vector<int> v { 1, 2, 3, 42, 321 };
    auto it = myfind(v, 42);
    assert(&*it == &v[3]);
}
