#if TEST_FUNCTION
 #include "function.h"  // Start by reading this file.
 #include "test-function.h"
#else
 #include "any.h"  // Next, edit this file.
 #include "any-cast.h"
 #include "test-any.h"
#endif

int main()
{
    run_tests();
}
