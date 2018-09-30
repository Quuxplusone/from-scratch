#include <cassert>
#include <future>
#include <thread>
#include <vector>

#define X std::this_thread::yield();

static std::atomic<int> destroyed_fizzbuzzes(0);

struct Fizzbuzz {
    int fizz;
    int buzz;
    ~Fizzbuzz() { destroyed_fizzbuzzes += 1; }
};


void test_refcounting_1()
{
    std::vector<std::thread> ts;
    my::shared_ptr<Fizzbuzz> p = my::make_shared<Fizzbuzz>();
    assert(destroyed_fizzbuzzes == 0);
    p = my::make_shared<Fizzbuzz>();
    assert(destroyed_fizzbuzzes == 1);
    for (int i=0; i < 30; ++i) {
X       ts.emplace_back([&](){
X           auto q = p;
X           my::shared_ptr<int> r(q, &q->fizz);
X           my::shared_ptr<int> s(q, &p->buzz);
X           q = my::shared_ptr<Fizzbuzz>(s, nullptr);
X           s = std::move(r);
X           r = my::shared_ptr<int>(p, &p->buzz);
X       });
X   }
X   assert(destroyed_fizzbuzzes == 1);
X   for (auto&& t : ts) t.join();
    assert(destroyed_fizzbuzzes == 1);
    p = nullptr;
    assert(destroyed_fizzbuzzes == 2);
}

void test_refcounting_2()
{
    std::thread tA, tB;
    std::promise<my::shared_ptr<Fizzbuzz>> a_to_b;
    std::promise<my::shared_ptr<Fizzbuzz>> b_to_a;

    for (int i=0; i < 100; ++i) {
        destroyed_fizzbuzzes = 0;
    
        tA = std::thread([&](){
X           auto ptr = my::make_shared<Fizzbuzz>();
X           a_to_b.set_value(ptr);
X           ptr = nullptr;
X           ptr = b_to_a.get_future().get();
X           b_to_a = decltype(b_to_a)();  // reset for the next round
X           ptr = nullptr;
        });

        tB = std::thread([&](){
X           auto ptr = a_to_b.get_future().get();
X           a_to_b = decltype(a_to_b)();  // reset for the next round
X           ptr = nullptr;
X           ptr = my::make_shared<Fizzbuzz>();
X           b_to_a.set_value(ptr);
X           ptr = nullptr;
        });

        tA.join();
        tB.join();
        assert(destroyed_fizzbuzzes == 2);
    }
}
