#include <cassert>
#include <cstdio>
#include <tuple>
#include "tuple-cat.h"

int main()
{
    auto first = std::tuple<int, double, char>(42, 3.14, 'x');
    auto second = std::tuple<>();
    auto third = std::tuple<float>(2.718f);
    auto my_result = my::tuple_cat(first, second, third);
    auto std_result = std::tuple_cat(first, second, third);
    static_assert(std::is_same_v<decltype(my_result), decltype(std_result)>);
    assert(my_result == std_result);
    puts("Success!");
}
