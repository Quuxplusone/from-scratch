#pragma once

#include <typeinfo>
#include <type_traits>

struct MyTypeInfo {
    void *(*convertToBase_)(char *p, const std::type_info& to);
    void *(*maybeFromHasAPublicChildOfTypeTo_)(char *p, int offset, const std::type_info& from, const std::type_info& to);
    bool (*isPublicBaseOfYourself)(int offset, const std::type_info& from);

    void *convertToBase(void *p, const std::type_info& to) const {
        return convertToBase_(reinterpret_cast<char*>(p), to);
    }
    void *maybeFromHasAPublicChildOfTypeTo(void *p, int offset, const std::type_info& from, const std::type_info& to) const {
        return maybeFromHasAPublicChildOfTypeTo_(reinterpret_cast<char*>(p), offset, from, to);
    }
};

void *dynamicast_to_mdo(void *p);
const std::type_info& dynamicast_typeid(void *mdo);
const MyTypeInfo& awkward_typeinfo_conversion(const std::type_info&);

#if defined(FREE_USE_OF_CXX17)

template<class PTo, class From>
PTo dynamicast(From *p) {
    using namespace std;
    static_assert(is_pointer_v<PTo>);
    using To = remove_pointer_t<PTo>;
    static_assert(is_polymorphic_v<From>);
    static_assert(is_polymorphic_v<To> || is_void_v<To>);

    if constexpr (is_void_v<To>) {
        void *mdo = dynamicast_to_mdo(p);
        return mdo;
    } else if constexpr (is_same_v<From, To>) {
        return p;
    } else if constexpr (is_base_of_v<To, From>) {
        // Technically we should also verify that To is an accessible base
        // in the current lexical scope, but we can't do that from in here.
        return (To *)(p);
    } else if constexpr (is_base_of_v<From, To>) {
        // dynamic_cast from base to derived
        void *mdo = dynamicast_to_mdo(p);
        const std::type_info& typeid_mdo = dynamicast_typeid(mdo);
        if (typeid_mdo == typeid(From)) {
            return nullptr;
        }
        const MyTypeInfo& ti = awkward_typeinfo_conversion(typeid_mdo);
        int offset = reinterpret_cast<char *>(p) - reinterpret_cast<char *>(mdo);
        if (typeid_mdo == typeid(To) && ti.isPublicBaseOfYourself(offset, typeid(From))) {
            return reinterpret_cast<To *>(mdo);
        } else if (void *subobj = ti.maybeFromHasAPublicChildOfTypeTo(mdo, offset, typeid(From), typeid(To))) {
            return reinterpret_cast<To *>(subobj);
        } else if (ti.isPublicBaseOfYourself(offset, typeid(From))) {
            return reinterpret_cast<To *>(ti.convertToBase(mdo, typeid(To)));
        }
        return nullptr;
    } else {
        // classes From and To are unrelated by inheritance
        if constexpr (is_final_v<To> || is_final_v<From>) {
            return nullptr;
        } else {
            void *mdo = dynamicast_to_mdo(p);
            const std::type_info& typeid_mdo = dynamicast_typeid(mdo);
            if (typeid_mdo == typeid(From)) {
                return nullptr;
            } else if (typeid_mdo == typeid(To)) {
                return reinterpret_cast<To *>(mdo);
            }
            const MyTypeInfo& ti = awkward_typeinfo_conversion(typeid_mdo);
            int offset = reinterpret_cast<char *>(p) - reinterpret_cast<char *>(mdo);
            if (ti.isPublicBaseOfYourself(offset, typeid(From))) {
                return reinterpret_cast<To *>(ti.convertToBase(mdo, typeid(To)));
            }
            return nullptr;
        }
    }
}

#else
#include "dynamicast-cxx14.h"
#endif
