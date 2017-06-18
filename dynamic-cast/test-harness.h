#include <cstdio>
#include <type_traits>

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

template<class T> static T *instance() { static T t; return &t; }

template<class F, class T, class = void> struct can_dynamic_cast_impl : std::false_type {};
template<class F, class T> struct can_dynamic_cast_impl<F, T, decltype(void(dynamic_cast<T>((F)nullptr)))> : std::true_type {};
template<class F, class T> struct can_dynamic_cast : can_dynamic_cast_impl<F, T, void> {};

template<bool B> using bool_if_t = std::enable_if_t<B, bool>;

inline int& failure_count() {
    static int count = 0;
    return count;
}

template<class To, class From>
void test(std::true_type, From *f, char *mdo, const char *stringification) {
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

template<class To, class From>
void test(std::false_type, From *, char *mdo, const char *stringification) {
    printf("NOOP: %s (%s)\n", __PRETTY_FUNCTION__, stringification);
}
