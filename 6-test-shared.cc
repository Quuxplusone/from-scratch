#include "future.h"
#include <assert.h>
#include <stdio.h>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

int main()
{
    my::promise<int> p;
    my::shared_future<int> f = p.get_future().share();

    std::mutex ready_mtx;
    std::unique_lock ready_lk(ready_mtx);
    std::thread tA([&, p = std::move(p)]() mutable {
        (void)std::lock_guard(ready_mtx);
        printf("Thread A is running...\n");
        std::this_thread::sleep_for(10ms);
        printf("Thread A is setting result to 42...\n");
        p.set_value(42);
    });
    std::thread tB([&, f]() mutable {
        (void)std::lock_guard(ready_mtx);
        printf("Thread B is running...\n");
        int result = f.get();
        printf("Thread B got result %d!\n", result);
    });
    std::thread tC([&, f = std::move(f)]() mutable {
        (void)std::lock_guard(ready_mtx);
        printf("Thread C is running...\n");
        int result = f.get();
        printf("Thread C got result %d!\n", result);
    });
    assert(!f.valid());
    ready_lk.unlock();
    tA.join();
    tB.join();
    tC.join();
}
