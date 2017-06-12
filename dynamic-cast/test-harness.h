#include <cstdio>
#include <type_traits>

template<class T> static T *instance = new T;

template<class F, class T, class = void> struct can_dynamic_cast_impl : std::false_type {};
template<class F, class T> struct can_dynamic_cast_impl<F, T, decltype(void(dynamic_cast<T*>((F*)nullptr)))> : std::true_type {};
template<class F, class T> struct can_dynamic_cast : can_dynamic_cast_impl<F, T, void> {};

template<class F, class T> constexpr bool can_dynamic_cast_v = can_dynamic_cast<F, T>::value;

template<bool B> using bool_if_t = std::enable_if_t<B, bool>;

inline int& failure_count() {
    static int count = 0;
    return count;
}

template<class To, class From, bool_if_t<can_dynamic_cast_v<From, To>> = true>
void test(From *f, char *mdo, const char *stringification) {
    To *p = dynamic_cast<To*>(f);
    To *q = dynamicast<To*>(f);
    if (p == q) {
        printf("PASS: %s (%s)\n", __PRETTY_FUNCTION__, stringification);
    } else {
        printf("FAIL: %s (%s)\n", __PRETTY_FUNCTION__, stringification);
        if (p && q) {
            printf("native = p+%d, dynamicast = p+%d\n", (int)((char*)p - mdo), (int)((char*)q - mdo));
        } else if (p) {
            printf("native = p+%d, dynamicast = NULL\n", (int)((char*)p - mdo));
        } else {
            printf("native = NULL, dynamicast = p+%d\n", (int)((char*)q - mdo));
        }
        failure_count() += 1;
    }
}

template<class To, class From, bool_if_t<!can_dynamic_cast_v<From, To>> = true>
void test(From *, char *mdo, const char *stringification) {
    printf("NOOP: %s (%s)\n", __PRETTY_FUNCTION__, stringification);
}
