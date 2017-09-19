#include <cassert>
#include <future>
#include <thread>
#include <vector>

#define Y std::this_thread::yield();

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
Y       ts.emplace_back([&](){
Y           auto q = p;
Y           my::shared_ptr<int> r(q, &q->fizz);
Y           my::shared_ptr<int> s(q, &p->buzz);
Y           q = my::shared_ptr<Fizzbuzz>(s, nullptr);
Y           s = std::move(r);
Y           r = my::shared_ptr<int>(p, &p->buzz);
Y       });
Y   }
Y   assert(destroyed_fizzbuzzes == 1);
Y   for (auto&& t : ts) t.join();
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
Y           auto ptr = my::make_shared<Fizzbuzz>();
Y           a_to_b.set_value(ptr);
Y           ptr = nullptr;
Y           ptr = b_to_a.get_future().get();
Y           b_to_a = decltype(b_to_a)();  // reset for the next round
Y           ptr = nullptr;
        });

        tB = std::thread([&](){
Y           auto ptr = a_to_b.get_future().get();
Y           a_to_b = decltype(a_to_b)();  // reset for the next round
Y           ptr = nullptr;
Y           ptr = my::make_shared<Fizzbuzz>();
Y           b_to_a.set_value(ptr);
Y           ptr = nullptr;
        });

        tA.join();
        tB.join();
        assert(destroyed_fizzbuzzes == 2);
    }
}
