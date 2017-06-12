#include <type_traits>
#include <cstdint>
#include <cstdio>
#include <string>
#include <typeinfo>

template<class U>
void dump_bytes(const U *vp)
{
    void *const *p = (void *const *)(void const *)vp;
    for (size_t i=0; i < sizeof(U); i += 8) {
        void *x = p[i/8];
        printf("%016llx", (uint64_t)x);
        printf("\n");
    }
}

std::string diagnose_typeinfo_pointer(const void *vp);

template<class U>
void dump_vtable(const U *vp, int vptr_offset)
{
    void *const *p = (void *const *)(void const *)vp;
    p = (void *const *)p[vptr_offset/8];
    for (int i=-10; i < 10; ++i) {
        void *x = p[i];
        printf("%016llx", (uint64_t)x);
        if (i == 0) printf(" *");
        if (i == -1) printf(" type_info pointer (%s)", diagnose_typeinfo_pointer(x).c_str());
        if (i == -2) printf(" offset to most-derived (%d)", (int)(uint64_t)x);
        if (i <= -3) printf(" offset to virtual base, or vcall offset");
        printf("\n");
    }
}

struct Animal {
    virtual ~Animal() {}
    int64_t animaldata = 0xA1A2A3A4;
};

struct Cat : virtual Animal {
    int64_t catdata = 0xC1C2C3C4;
};

struct Dog : virtual Animal {
    int64_t dogdata = 0xD1D2D3D4;
};

struct CatDog : Cat, Dog {
    int64_t cddata = 0xCDCDCDCD;
};

struct Rat : Animal {
    int64_t ratdata = 0xEEEEEEEE;
};

struct RatCatDog : Rat, CatDog {
    int64_t ratdata = 0xECDECDECD;
};

std::string diagnose_typeinfo_pointer(const void *vp)
{
    const std::type_info *p = (const std::type_info *)vp;
    if (!dynamic_cast<const std::type_info *>(p)) {
        return "not a typeinfo pointer";
#define T(C) \
    } else if (p == &typeid(C)) { \
        return #C;
T(Animal)
T(Cat)
T(Dog)
T(CatDog)
T(Rat)
T(RatCatDog)
    } else {
        return "unknown typeinfo pointer";
    }
}

int main()
{
    Animal a;
    dump_bytes(&a);
    printf("-------------\n");
    dump_vtable(&a, 0);
    printf("-------------\n");
    Cat c;
    dump_bytes(&c);
    printf("-------------\n");
    dump_vtable(&c, 0);
    printf("-------------\n");
    dump_vtable(&c, 16);
    printf("-------------\n");
}
