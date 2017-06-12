
#include "dynamicast.h"
#include "catdog.h"

#include <cassert>
#include <typeinfo>

//////////////////////////////////////////////////////////////////// Animal
void *Animal_convertToBase(char *p, const std::type_info& to) {
    assert(false);  // I have no bases.
    return nullptr;
}
void *Animal_maybeFromHasAPublicChildOfTypeTo(char *p, int offset, const std::type_info& from, const std::type_info& to) {
    return nullptr;
}
bool Animal_isPublicBaseOfYourself(int offset, const std::type_info& from) {
    assert(false);  // I have no bases.
    return true;
}
MyTypeInfo Animal_typeinfo {
    Animal_convertToBase,
    Animal_maybeFromHasAPublicChildOfTypeTo,
    Animal_isPublicBaseOfYourself,
};

/////////////////////////////////////////////////////////////////////// Cat
void *Cat_convertToBase(char *p, const std::type_info& to) {
    if (to == typeid(Animal)) return p + 16;
    return nullptr;
}
void *Cat_maybeFromHasAPublicChildOfTypeTo(char *p, int offset, const std::type_info& from, const std::type_info& to) {
    return nullptr;
}
bool Cat_isPublicBaseOfYourself(int offset, const std::type_info& from) {
    return true;  // I have only public bases.
}
MyTypeInfo Cat_typeinfo {
    Cat_convertToBase,
    Cat_maybeFromHasAPublicChildOfTypeTo,
    Cat_isPublicBaseOfYourself,
};

/////////////////////////////////////////////////////////////////////// Dog
void *Dog_convertToBase(char *p, const std::type_info& to) {
    if (to == typeid(Animal)) return p + 16;
    return nullptr;
}
void *Dog_maybeFromHasAPublicChildOfTypeTo(char *p, int offset, const std::type_info& from, const std::type_info& to) {
    return nullptr;
}
bool Dog_isPublicBaseOfYourself(int offset, const std::type_info& from) {
    return true;  // I have only public bases.
}
MyTypeInfo Dog_typeinfo {
    Dog_convertToBase,
    Dog_maybeFromHasAPublicChildOfTypeTo,
    Dog_isPublicBaseOfYourself,
};

//////////////////////////////////////////////////////////////////// CatDog
void *CatDog_convertToBase(char *p, const std::type_info& to) {
    if (to == typeid(Cat)) return p + 0;
    if (to == typeid(Dog)) return p + 16;
    if (to == typeid(Animal)) return p + 32;
    return nullptr;
}
void *CatDog_maybeFromHasAPublicChildOfTypeTo(char *p, int offset, const std::type_info& from, const std::type_info& to) {
    if (from == typeid(Cat) && to == typeid(Animal)) return p + 32;
    if (from == typeid(Dog) && to == typeid(Animal)) return p + 32;
    return nullptr;
}
bool CatDog_isPublicBaseOfYourself(int offset, const std::type_info& from) {
    return true;  // I have only public bases.
}
MyTypeInfo CatDog_typeinfo {
    CatDog_convertToBase,
    CatDog_maybeFromHasAPublicChildOfTypeTo,
    CatDog_isPublicBaseOfYourself,
};

///////////////////////////////////////////////////////////////////// Coral
void *Coral_convertToBase(char *p, const std::type_info& to) {
    // Animal is a protected base of Coral, so it does not appear here.
    return nullptr;
}
void *Coral_maybeFromHasAPublicChildOfTypeTo(char *p, int offset, const std::type_info& from, const std::type_info& to) {
    return nullptr;
}
bool Coral_isPublicBaseOfYourself(int offset, const std::type_info& from) {
    return false;  // I have only non-public bases.
}
MyTypeInfo Coral_typeinfo {
    Coral_convertToBase,
    Coral_maybeFromHasAPublicChildOfTypeTo,
    Coral_isPublicBaseOfYourself,
};

////////////////////////////////////////////////////////////////////// Fish
void *Fish_convertToBase(char *p, const std::type_info& to) {
    if (to == typeid(Animal)) return p + 0;
    return nullptr;
}
void *Fish_maybeFromHasAPublicChildOfTypeTo(char *p, int offset, const std::type_info& from, const std::type_info& to) {
    return nullptr;
}
bool Fish_isPublicBaseOfYourself(int offset, const std::type_info& from) {
    return true;  // I have only public bases.
}
MyTypeInfo Fish_typeinfo {
    Fish_convertToBase,
    Fish_maybeFromHasAPublicChildOfTypeTo,
    Fish_isPublicBaseOfYourself,
};

////////////////////////////////////////////////////////////////////// Nemo
void *Nemo_convertToBase(char *p, const std::type_info& to) {
    if (to == typeid(Fish)) return p + 0;
    if (to == typeid(Coral)) return p + 16;
    // Animal is an ambiguous base, so it is not listed here.
    return nullptr;
}
void *Nemo_maybeFromHasAPublicChildOfTypeTo(char *p, int offset, const std::type_info& from, const std::type_info& to) {
    if (from == typeid(Fish) && to == typeid(Animal) && offset == 0) return p + 0;
    return nullptr;
}
bool Nemo_isPublicBaseOfYourself(int offset, const std::type_info& from) {
    if (from == typeid(Animal) && offset == 0) return true;
    if (from == typeid(Fish) && offset == 0) return true;
    if (from == typeid(Animal) && offset == 16) return false;
    if (from == typeid(Coral) && offset == 16) return true;
    printf("unexpectedly %d %s\n", offset, from.name());
    assert(false);
    return false;
}
MyTypeInfo Nemo_typeinfo {
    Nemo_convertToBase,
    Nemo_maybeFromHasAPublicChildOfTypeTo,
    Nemo_isPublicBaseOfYourself,
};

//////////////////////////////////////////////////////////////// dispatcher
const MyTypeInfo& awkward_typeinfo_conversion(const std::type_info& ti)
{
#define CASE(Cat) if (ti == typeid(Cat)) return Cat##_typeinfo
    CASE(Animal);
    CASE(Cat);
    CASE(Dog);
    CASE(CatDog);
    CASE(Coral);
    CASE(Fish);
    CASE(Nemo);
    assert(false);
}
