#include <cassert>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace my {

template<class It>
using iterator_value_t = typename std::iterator_traits<It>::value_type;

template<class It>
It find(It first, It last, const iterator_value_t<It>& value)
{
    while (first != last) {
        if (*first == value) {
            return first;
        }
        ++first;
    }
    return last;
}

} // namespace my

int main()
{
    std::vector<std::string> v = {
        "the", "quick", "brown", "fox", "jumped", "over", "the", "lazy", "dog"
    };
    auto testcase = my::find(v.begin(), v.end(), "hello");
    assert(testcase == v.end());
}
