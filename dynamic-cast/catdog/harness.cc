#include <cstdio>
#include <type_traits>

#include "catdog.h"
#include "dynamicast.h"

template<class F, class T, class = void> struct can_dynamic_cast_impl : std::false_type {};
template<class F, class T> struct can_dynamic_cast_impl<F, T, decltype(void(dynamic_cast<T*>((F*)nullptr)))> : std::true_type {};
template<class F, class T> struct can_dynamic_cast : can_dynamic_cast_impl<F, T, void> {};

template<class F, class T> constexpr bool can_dynamic_cast_v = can_dynamic_cast<F, T>::value;

template<bool B> using bool_if_t = std::enable_if_t<B, bool>;


template<class To, class From, bool_if_t<can_dynamic_cast_v<From, To>> = true>
void test(From *f) {
    printf("%s", __PRETTY_FUNCTION__);
    To *p = dynamic_cast<To*>(f);
    To *q = dynamicast<To*>(f);
    if (p == q) {
        printf("\rPASS: %s\n", __PRETTY_FUNCTION__);
    } else {
        printf("\rFAIL: %s\n", __PRETTY_FUNCTION__);
        printf("expected=%p, result=%p\n", p, q);
    }
}

template<class To, class From, bool_if_t<!can_dynamic_cast_v<From, To>> = true>
void test(From *) {
    printf("NOOP: %s\n", __PRETTY_FUNCTION__);
}

template<class T> static T *instance = new T;

template<class To>
void test_to() {
    test<To>(instance<Animal>->as_animal());

    test<To>(instance<Cat>->as_animal());
    test<To>(instance<Cat>->as_cat());

    test<To>(instance<Dog>->as_animal());
    test<To>(instance<Dog>->as_dog());

    test<To>(instance<CatDog>->as_animal());
    test<To>(instance<CatDog>->as_cat());
    test<To>(instance<CatDog>->as_dog());

    test<To>(instance<Coral>->as_animal());
    test<To>(instance<Coral>->as_coral());

    test<To>(instance<Fish>->as_animal());
    test<To>(instance<Fish>->as_fish());

    test<To>(instance<Nemo>->as_coral()->as_animal());
    test<To>(instance<Nemo>->as_fish()->as_animal());
    test<To>(instance<Nemo>->as_coral());
    test<To>(instance<Nemo>->as_fish());
    test<To>(instance<Nemo>->as_nemo());
}

int main()
{
    test_to<void>();
    test_to<Animal>();
    test_to<Cat>();
    test_to<Dog>();
    test_to<CatDog>();
    test_to<Coral>();
    test_to<Fish>();
    test_to<Nemo>();
}
