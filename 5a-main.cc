#include "shared-ptr.h"
#include "make-shared.h"

// #include "test-crtp.cc"
#include "test-refcounting.cc"

int main()
{
    test_refcounting_1();
    test_refcounting_2();
    // test_crtp();
}
