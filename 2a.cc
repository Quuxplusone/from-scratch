#include <stdio.h>
#include <functional>

int g = 0;

template<class F>
void test(F f)
{
    printf("%zu\t", sizeof f);
    g = 10;
    printf("%d\t", f());
    g = 20;
    printf("%d\n", f());
}

void test() { puts(""); }

int main()
{
    char x [[maybe_unused]] = 1;
    int y [[maybe_unused]] = 2;
    int z [[maybe_unused]] = 0;

#if GRADUALLY_MOVE_THIS_PREPROCESSOR_DIRECTIVE_DOWNWARDS_TO_REVEAL_TEST_CASES

    test(  []() { return 1; }                       );
    test(                                           );
    test(  []() { return g; }                       );
    test(                                           );    
    test(  [a = g]() { return a + 1; }              );
    test(                                           );
    test(  [x]() { return x; }                      );
    test(  [=]() { return x; }                      );
    test(  [=]() { return x + y; }                  );
    test(                                           );
    test(  [&x]() { return x; }                     );
    test(  [&]() { return x; }                      );
    test(  [&]() { return x + y; }                  );
    test(                                           );
    test(                                                  );
    test(  []() { int a = 0; return ++a; }                 );
    test(  []() { static int a = 0; return ++a; }          );
    test(  [a = 0]() mutable { return ++a; }               );
    test(                                                  );
    test(  [](int a = 3) { return a; }                     );
    test(  [](auto... a) -> int { return sizeof...(a); }   );
    test(                                                  );
    test(  [&z = z]() { return z != 0; }                   );
    test(  [z = &z]() { return z != 0; }                   );

#endif // DONE!
}
