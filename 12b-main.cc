#include <cassert>
#include <set>
#include "Person.h"
#include "PersonLessThan.h"

int main()
{
    my::Person alice("Alice", "Adams", 37);
    my::Person bob("Bob", "Jones", 21);

    std::set<my::Person, PersonLessThan> s;
    s.insert(alice);
    s.insert(bob);
    assert(s.size() == 2);
    
    // BONUS:
    assert(s.count("Alice Adams") == 1);
}
