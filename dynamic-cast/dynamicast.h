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

void *dynamicast_get_mdo(void *p);
const std::type_info& dynamicast_get_std_typeinfo(void *mdo);
const MyTypeInfo& awkward_typeinfo_conversion(const std::type_info&);

#ifdef FREE_USE_OF_CXX17

template<class PTo, class From>
PTo dynamicast(From *p) {
    using namespace std;
    static_assert(is_pointer_v<PTo>);
    using To = remove_pointer_t<PTo>;
    static_assert(is_polymorphic_v<From>);
    static_assert(is_polymorphic_v<To> || is_void_v<To>);

    if constexpr (is_void_v<To>) {
        void *mdo = dynamicast_get_mdo(p);
        return mdo;
    } else if constexpr (is_same_v<From, To>) {
        return p;
    } else if constexpr (is_base_of_v<To, From>) {
        // Technically we should also verify that To is an accessible base
        // in the current lexical scope, but we can't do that from in here.
        return (To *)(p);
    } else if constexpr (is_base_of_v<From, To>) {
        // dynamic_cast from base to derived
        void *mdo = dynamicast_get_mdo(p);
        const std::type_info& typeid_mdo = dynamicast_get_std_typeinfo(mdo);
        if (typeid_mdo == typeid(From)) {
            return nullptr;
        }
        const MyTypeInfo& ti = awkward_typeinfo_conversion(typeid_mdo);
        int offset = reinterpret_cast<char *>(p) - reinterpret_cast<char *>(mdo);
        if (typeid_mdo == typeid(To) && ti.isPublicBaseOfYourself(offset, typeid(From))) {
            return reinterpret_cast<To *>(mdo);
        }
        void *subobj = ti.maybeFromHasAPublicChildOfTypeTo(mdo, offset, typeid(From), typeid(To));
        if (subobj != nullptr) {
            return reinterpret_cast<To *>(subobj);
        } else if (ti.isPublicBaseOfYourself(offset, typeid(From))) {
            return reinterpret_cast<To *>(ti.convertToBase(mdo, typeid(To)));
        } else {
            return nullptr;
        }
    } else {
        // classes From and To are unrelated by inheritance
        if constexpr (is_final_v<To> || is_final_v<From>) {
            return nullptr;
        } else {
            void *mdo = dynamicast_get_mdo(p);
            int offset = reinterpret_cast<char *>(p) - reinterpret_cast<char *>(mdo);
            const std::type_info& typeid_mdo = dynamicast_get_std_typeinfo(mdo);
            if (typeid_mdo == typeid(From)) {
                return nullptr;
            } else if (typeid_mdo == typeid(To)) {
                return reinterpret_cast<To *>(mdo);
            }
            const MyTypeInfo& ti = awkward_typeinfo_conversion(typeid_mdo);
            if (!ti.isPublicBaseOfYourself(offset, typeid(From))) {
                return nullptr;
            }
            return reinterpret_cast<To *>(ti.convertToBase(mdo, typeid(To)));
        }
    }
}

#else

template<class T, class U> constexpr bool is_base_of_v = std::is_base_of<T,U>::value;
template<class T> constexpr bool is_final_v = std::is_final<T>::value;
template<class T> constexpr bool is_pointer_v = std::is_pointer<T>::value;
template<class T> constexpr bool is_polymorphic_v = std::is_polymorphic<T>::value;
template<class T, class U> constexpr bool is_same_v = std::is_same<T,U>::value;
template<class T> constexpr bool is_void_v = std::is_void<T>::value;

template<bool B> using bool_if_t = std::enable_if_t<B, bool>;

template<size_t I> struct priority_tag : priority_tag<I-1> {};
template<> struct priority_tag<0> {};

template<class From, class To, bool_if_t<is_void_v<To>> = true>
To *dynamicast_impl(From *p, priority_tag<9>) {
    void *mdo = dynamicast_get_mdo(p);
    return mdo;
}

template<class From, class To, bool_if_t<is_same_v<From, To>> = true>
To *dynamicast_impl(From *p, priority_tag<8>) {
    return p;
}

template<class From, class To, bool_if_t<is_base_of_v<To, From>> = true>
To *dynamicast_impl(From *p, priority_tag<7>) {
    // Technically we should also verify that To is an accessible base
    // in the current lexical scope, but we can't do that from in here.
    return (To *)(p);
}

template<class From, class To, bool_if_t<is_base_of_v<From, To>> = true>
To *dynamicast_impl(From *p, priority_tag<6>) {
    // dynamic_cast from base to derived
    void *mdo = dynamicast_get_mdo(p);
    const std::type_info& typeid_mdo = dynamicast_get_std_typeinfo(mdo);
    if (typeid_mdo == typeid(From)) {
        return nullptr;
    }
    const MyTypeInfo& ti = awkward_typeinfo_conversion(typeid_mdo);
    int offset = reinterpret_cast<char *>(p) - reinterpret_cast<char *>(mdo);
    if (typeid_mdo == typeid(To) && ti.isPublicBaseOfYourself(offset, typeid(From))) {
        return reinterpret_cast<To *>(mdo);
    }
    void *subobj = ti.maybeFromHasAPublicChildOfTypeTo(mdo, offset, typeid(From), typeid(To));
    if (subobj != nullptr) {
        return reinterpret_cast<To *>(subobj);
    } else if (ti.isPublicBaseOfYourself(offset, typeid(From))) {
        return reinterpret_cast<To *>(ti.convertToBase(mdo, typeid(To)));
    } else {
        return nullptr;
    }
}

template<class From, class To, bool_if_t<is_final_v<To> || is_final_v<From>> = true>
To *dynamicast_impl(From *, priority_tag<5>) {
    // classes From and To are unrelated by inheritance
    return nullptr;
}

template<class From, class To, bool_if_t<true> = true>
To *dynamicast_impl(From *p, priority_tag<4>) {
    // classes From and To are unrelated by inheritance
    void *mdo = dynamicast_get_mdo(p);
    int offset = reinterpret_cast<char *>(p) - reinterpret_cast<char *>(mdo);
    const std::type_info& typeid_mdo = dynamicast_get_std_typeinfo(mdo);
    if (typeid_mdo == typeid(From)) {
        return nullptr;
    } else if (typeid_mdo == typeid(To)) {
        return reinterpret_cast<To *>(mdo);
    }
    const MyTypeInfo& ti = awkward_typeinfo_conversion(typeid_mdo);
    if (!ti.isPublicBaseOfYourself(offset, typeid(From))) {
        return nullptr;
    }
    return reinterpret_cast<To *>(ti.convertToBase(mdo, typeid(To)));
}

template<class PTo, class From>
PTo dynamicast(From *p) {
    using namespace std;
    static_assert(is_pointer_v<PTo>);
    using To = std::remove_pointer_t<PTo>;
    static_assert(is_polymorphic_v<From>);
    static_assert(is_polymorphic_v<To> || is_void_v<To>);

    return dynamicast_impl<From, To>(p, priority_tag<9>{});
}

#endif
