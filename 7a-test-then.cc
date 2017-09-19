#include "future.h"
#include <assert.h>
#include <stdio.h>

// Test the basic functionality of "then".
void run_basic_test()
{
    auto twice = [](auto x) { return 2 * x.get(); };
    auto plus = [](auto k) { return [k](auto x){ return x.get() + k; }; };

    my::promise<int> p;
    my::future<int> f = p.get_future().then(twice).then(plus(3));
    p.set_value(10);
    int x = f.get();
    assert(x == 10*2 + 3);
    printf("Ok 1!\n");
}

// Test our shorthand "next".
void run_next_test()
{
    auto twice = [](auto x) { return 2 * x; };
    auto plus = [](auto k) { return [k](auto x){ return x + k; }; };

    my::promise<int> p;
    my::future<int> f = p.get_future().next(twice).next(plus(3));
    p.set_value(10);
    int x = f.get();
    assert(x == 10*2 + 3);
    printf("Ok 2!\n");
}

// Test that the continuation gets executed if it's attached to an already ready future.
void run_ready_test()
{
    auto twice = [](auto x) { return 2 * x; };
    auto plus = [](auto k) { return [k](auto x){ return x + k; }; };

    my::promise<int> p;
    p.set_value(10);
    my::future<int> f = p.get_future().next(twice).next(plus(3));
    int x = f.get();
    assert(x == 10*2 + 3);
    printf("Ok 3!\n");
}

// Test what happens if the promise is satisfied by an exception.
void run_exception_test()
{
    auto twice = [](auto x) { return x == 10 ? throw std::logic_error("Ok 4!") : 2 * x; };
    auto handle_exception = [](std::exception_ptr ex) {
        try {
            std::rethrow_exception(ex);
        } catch (const std::exception& ex) {
            puts(ex.what());
        }
        return -99;
    };

    // Test the exceptional codepath.
    my::promise<int> p;
    my::future<int> f = p.get_future().next(twice).next(twice).recover(handle_exception);
    p.set_value(10);
    assert(f.get() == -99);

    // Prepare a new promise with a new shared state. Test the non-exceptional codepath.
    p = decltype(p){};
    f = p.get_future().next(twice).next(twice).recover(handle_exception);
    p.set_value(11);
    assert(f.get() == 11*2*2);
}

// Test that the race between then() and set_ready() is handled correctly.
void run_race_test()
{
    for (int i=0; i < 100; ++i) {
        my::promise<int> p;
        my::future<int> f = p.get_future();
        std::thread tB([f = std::move(f)]() mutable {
            for (int i=0; i < 10; ++i) {
                f = f.then([](auto f) { int x = f.get(); return x + 1; });
            }
            assert(f.get() == 20);
        });
        std::thread tA([p = std::move(p)]() mutable {
            p.set_value(10);
        });
        tA.join();
        tB.join();
    }
    printf("Ok 5!\n");
}
