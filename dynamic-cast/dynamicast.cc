#include "dynamicast.h"

#include <cstdint>

#ifdef _MSC_VER // 64-bit MSVC ABI

void *dynamicast_to_mdo(void *p)
{
    if (((int**)p)[0][0] == 0) {
        p = (char *)p + ((int**)p)[0][1];
    }
    int *complete_object_locator = ((int ***)p)[0][-1];
    int mdoffset = complete_object_locator[1];
    void *adjusted_this = static_cast<char *>(p) - mdoffset;
    return adjusted_this;
}

const std::type_info& dynamicast_typeid(void *p)
{
    if (((int**)p)[0][0] == 0) {
        p = (char *)p + ((int**)p)[0][1];
    }
    int *complete_object_locator = ((int ***)p)[0][-1];

    char *result = (char *)complete_object_locator;
    result -= complete_object_locator[5];
    result += complete_object_locator[3];
    return *(const std::type_info*)result;
}

#else // Itanium ABI

void *dynamicast_to_mdo(void *p)
{
    uint64_t *vptr = *reinterpret_cast<uint64_t **>(p);
    uint64_t mdoffset = vptr[-2];
    void *adjusted_this = static_cast<char *>(p) + mdoffset;
    return adjusted_this;
}

const std::type_info& dynamicast_typeid(void *p)
{
    std::type_info **vptr = *reinterpret_cast<std::type_info ***>(p);
    std::type_info *typeinfo_ptr = vptr[-1];
    return *typeinfo_ptr;
}

#endif
