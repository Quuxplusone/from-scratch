#pragma once

#include <typeinfo>
#include <type_traits>

template<class From, class To, class B, class C, class D>
To *dynamicast_impl(From *p, std::true_type, B, C, D) {
    void *mdo = dynamicast_to_mdo(p);
    return mdo;
}

template<class From, class To, class C, class D>
To *dynamicast_impl(From *p, std::false_type, std::true_type, C, D) {
    return p;
}

template<class From, class To, class D>
To *dynamicast_impl(From *p, std::false_type, std::false_type, std::true_type, D) {
    // Technically we should also verify that To is an accessible base
    // in the current lexical scope, but we can't do that from in here.
    return (To *)(p);
}

template<class From, class To>
To *dynamicast_impl(From *p, std::false_type, std::false_type, std::false_type, std::true_type) {
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
    } else {
        return nullptr;
    }
}

template<class From, class To>
To *dynamicast_impl(From *p, std::false_type, std::false_type, std::false_type, std::false_type) {
    // classes From and To are unrelated by inheritance
    if (std::is_final<From>::value || std::is_final<To>::value) {
        return nullptr;
    }
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

template<class PTo, class From>
PTo dynamicast(From *p) {
    using namespace std;
    static_assert(is_pointer<PTo>::value, "Argument to dynamicast must be a pointer");
    using To = remove_pointer_t<PTo>;
    static_assert(is_polymorphic<From>::value, "Argument to dynamicast must be a polymorphic type");
    static_assert(is_polymorphic<To>::value || is_void<To>::value, "Template parameter to dynamicast must be a polymorphic type or void");

    return dynamicast_impl<From, To>(p,
        std::is_void<To>{},
        std::is_same<From, To>{},
        std::is_base_of<To, From>{},
        std::is_base_of<From, To>{}
    );
}
