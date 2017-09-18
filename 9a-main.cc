#include <cassert>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace my {
#include "find.h"
#include "count.h"
} // namespace my

int main()
{
    std::vector<std::string> v = {
        "the", "quick", "brown", "fox", "jumped", "over", "the", "lazy", "dog"
    };
    auto contains = [](char ch) {
        return [ch](auto&& word) {
            return word.find(ch) != std::string::npos;
        };
    };
    assert(my::find(v.begin(), v.end(), "the") == v.begin());
    assert(my::find(v.begin(), v.end(), "fox") == v.begin() + 3);
    assert(my::find(v.begin(), v.end(), "hello") == v.end());
    assert(my::find_if(v.begin(), v.end(), contains('o')) == v.begin() + 2);
    assert(my::find_if(v.begin(), v.end(), contains('s')) == v.end());

    assert(my::count(v.begin(), v.end(), "the") == 2);
    assert(my::count(v.begin(), v.end(), "fox") == 1);
    assert(my::count(v.begin(), v.end(), "hello") == 0);
    assert(my::count_if(v.begin(), v.end(), contains('o')) == 4);
    assert(my::count_if(v.begin(), v.end(), contains('s')) == 0);
}

