This repository contains "from scratch" implementations of many C++17
standard library features. It's intended for use with my upcoming
workshop on "The Standard Library From Scratch".

This code is *not* intended for use in production!

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT.

(That is, I won't stop you from using it, but it's liable to be
inefficient, incomplete, incorrect, non-portable, and so on.
Use the actual standard library instead; it's what it's there for.)


The good stuff
--------------

If you're looking for the code snippets, you should skip straight
to [the `bits/` subdirectory](include/scratch/bits/).


`git log` is your friend
------------------------

The git history of this repo is just as much a part of the "product"
as the code at the current top-of-tree. I'll try to keep the commits
topical, and if I find out that something in the history is wrong,
I'll go fix it. This means that *this repo will be force-pushed a lot.*

If you're browsing the code, all you need to know is that `git log`
and `git blame` will be very useful to you.


Writing test cases
------------------

In general, you should be able to compile a test program with

    g++ -std=c++14 -I ${THIS_REPO}/include/ test.cc

The "from scratch" library should behave pretty much the same as the
standard library; just do a global search-and-replace on your
`#include`s and your `std`s.

    #include <scratch/algorithm>

    int main() {
        scratch::vector<int> v{3, 1, 4, 1, 5, 9};
        scratch::sort(v.begin(), v.end());
        assert((v == scratch::vector<int>{1, 1, 3, 4, 5, 9}));
    }
