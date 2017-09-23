#pragma once

struct CStringHash
{
    size_t operator()(const char *) const {
        //
        // YOUR CODE GOES HERE
        //
        return 0;
    }
    bool operator()(const char *, const char *) const {
        //
        // YOUR CODE GOES HERE
        //
        return true;
    }
};
