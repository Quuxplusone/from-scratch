#include <cassert>
#include <typeinfo>

struct Large {
    int m_big[1000];
    int m_x;
    Large(int x) : m_x(x) {}
};

void run_tests()
{
    // Test simple cases.

    any f = 42;
    assert(f.has_value());
    assert(any_cast<int>(f) == 42);

    f = 3.14;
    assert(f.has_value());
    assert(any_cast<double>(f) == 3.14);

    // Test that copy-assignment does the right thing.

    any f2 = 42;
    assert(f2.type() == typeid(int));
    f2 = f;
    assert(f2.type() == typeid(double));

    // Test that arguments and return values work.

    any g = 42.0;
    assert(any_cast<double>(g) == 42.0);
    
    // Test that very large objects work.
    
    any g2 = Large(1);
    g = g2;
    g = Large(2);
    
    // Test type-unerasure.

    assert(any_cast<Large>(&g) != nullptr && any_cast<Large&>(g).m_x == 2);
    assert(any_cast<Large>(&std::as_const(g)) != nullptr && any_cast<const Large&>(g).m_x == 2);
    
    g = any{};
    assert(!g.has_value());
    assert(g.type() == typeid(void) && any_cast<Large>(&g) == nullptr);

    // Test conversion from an empty any (versus a null pointer).

    g = nullptr;
    assert(g.has_value() && g.type() == typeid(nullptr));
    g = any{};
    assert(!g.has_value());
    g = static_cast<int (*)(int, int)>(nullptr);
    assert(g.has_value() && g.type() == typeid(int (*)(int, int)));
}
