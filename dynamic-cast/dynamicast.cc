#include "dynamicast.h"

#include <cstdint>

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
