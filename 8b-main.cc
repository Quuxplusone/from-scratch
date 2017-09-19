#include "unique-function.h"
#include <cstdio>
#include <functional>

int main()
{
    unique_function<void()> f = [](){ printf("hello from %s\n", __PRETTY_FUNCTION__); };
    f();  // prints "hello"
    f = [](){ printf("goodbye from %s\n", __PRETTY_FUNCTION__); };
    f(); // prints "goodbye"
    unique_function<int()> g = [f = std::move(f)]() mutable { f(); return 42; };
    f = [g = std::move(g)]() mutable { if (g() == 42) puts("Hooray!"); };
    f();  // prints "goodbye" from the nested captured f, then "Hooray!"
    try {
        g();  // g has been moved-out-of, so it should throw
    } catch (const std::bad_function_call&) {
        puts("g was nulled out as expected.");
    }
}
