#include <cstdio>
#include <functional>
#include <string>
#include <unordered_map>
#include "CStringHash.h"

int main()
{
    char hello1[] = "hello", hello2[] = "hello";
    char world1[] = "world", world2[] = "world";

    std::unordered_map<const char *, int, CStringHash> m;
    m[hello1] = 1;
    m[hello2] = 2;  // We'd like this to overwrite the entry "hello:1" with "hello:2"
    m[world1] = 3;
    m[world2] = 4;  // We'd like this to overwrite the entry "world:3" with "world:4"

    for (auto&& kv : m) {
        printf("%s: %d\n", kv.first, kv.second);  // should print "hello: 2 \n world: 4" or vice versa
    }
}
