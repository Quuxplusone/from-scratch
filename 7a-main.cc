#include "future.h"
#include "test-then.cc"

int main()
{
    run_basic_test();      // You should see "Ok 1!" printed.
    run_next_test();       // You should see "Ok 2!" printed.
    run_ready_test();      // You should see "Ok 3!" printed.
    run_exception_test();  // You should see "Ok 4!" printed (just once).
    run_race_test();       // You should see "Ok 5!" printed.
}
