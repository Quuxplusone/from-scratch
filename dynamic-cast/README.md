`dynamic_cast` from scratch
---------------------------

This directory contains materials related to my upcoming CppCon presentation
"`dynamic_cast` From Scratch." This includes a complete implementation of
the semantics of `dynamic_cast` in both C++17 and C++14.

My `dynamic_cast` implementation, which I'm calling `dynamicast<To>()`,
uses its own "type info" which is not compatible with the typeinfo produced
by compilers conforming to the Itanium ABI.

The actual typeinfo consists of three short functions and one struct
tying them together into a bundle of function pointers:

    #include "dynamicast.h"
    void *Nemo_convertToBase(char *mdo, const std::type_info& to);
    void *Nemo_maybeFromHasAPublicChildOfTypeTo(char *mdo, int offset, const std::type_info& from, const std::type_info& to);
    bool Nemo_isPublicBaseOfYourself(int offset, const std::type_info& from);
    MyTypeInfo Nemo_typeinfo {
        Nemo_convertToBase,
        Nemo_maybeFromHasAPublicChildOfTypeTo,
        Nemo_isPublicBaseOfYourself,
    };

`Nemo_convertToBase` is used by `dynamicast` and also by `catch`, when the
exception being unwound is of type `Nemo`; it casts a most-derived-object
to one of its public base classes.

`Nemo_maybeFromHasAPublicChildOfTypeTo` takes the address and type of the
current base subobject (`from`) and the name of a destination type, and
calculates whether the given base subobject can be converted via `dynamic_cast`
to a derived subobject of type `to`. If so, return a pointer to that derived
subobject; if not, return `nullptr`. Notice that this function may return
non-null even if `Nemo_convertToBase(mdo, to)` would have returned null —
this can happen due to ambiguous bases, or due to `to`'s being a non-public
base class of `Nemo`.

`Nemo_isPublicBaseOfYourself` takes the address and type of the current base
subobject (`from`) and tells whether `from` is a public base of the most
derived object. Notice that just taking the *type* would not be good enough,
since the most derived object might have multiple base subobjects of type
`from`, some of which are public and some of which aren't.

Because the typeinfo pointer stored in the vtable points to the *standard*
typeinfo, we glue things together via an awkward runtime mapping from standard
`std::type_info` pointers (returned by `typeid`) to our own `MyTypeInfo`
pointers (containing the information needed for `dynamicast`).

    const MyTypeInfo& awkward_typeinfo_conversion(const std::type_info& ti);

Of course, if this way of doing `dynamic_cast` were standardized, then the
compiler would be the one generating the typeinfo routines, `MyTypeInfo` would
be stored in the vtable, and there would be no need for the
`awkward_typeinfo_conversion` function at all.


Simple CatDog example
---------------------

The directory `catdog/` contains a few simple examples of class layouts
and how to hand-write typeinfo for them. It also contains a C++ program
`dump-vtable.cc` that can be used to explore the Itanium ABI vtable layout.


`dynamic_cast` fuzzer
---------------------

This directory contains a Python script `generate-harness.py`
that does the following:

- generate a random set of C++ classes into `things.gen.h`
- generate typeinfo for those classes into `things.gen.cc`
- generate a testing harness for all possible cast operations into `harness.gen.cc`

Running `make local SEED=42` will run the generator with
a specific seed and then compile the resulting harness into `./fuzz`.

Running `make clang SEED=42` will run the generator with a specific seed
and then submit the result to `clang` on Wandbox. (This depends on
`../dependency-graph/unity-dump.py`.) Likewise `make gcc` will submit
to `g++` on Wandbox. You probably want to pipe the output through grep:

    make clang SEED=42 | grep -A1 -i fail

Running `./find-bugs.py --seed=42` will run `make gcc` and `make clang`
on seeds starting at 42 and increasing without limit. All output will be
suppressed unless a bug is detected, in which case the seed and the compiler
will be printed. There are many cases where the output is `both`, because
a single complicated test case triggers independent bugs in both Clang and GCC.


Existing `dynamic_cast` implementations are buggy and slow
----------------------------------------------------------

In working on the fuzzer, I found the following bugs in libc++abi (the LLVM
project's C++ support library) and libsupc++ (the GNU project's C++ support
library):

* https://bugs.llvm.org/show_bug.cgi?id=33425
* https://bugs.llvm.org/show_bug.cgi?id=33439
* https://bugs.llvm.org/show_bug.cgi?id=33487
* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81078

It's difficult to emulate any of these bugs in my code — the correct code
is so simple and has so little room for error that it's difficult to inject
error even if you want to. However, I've managed to emulate bugs 33425
and 33439 in the `dynamicast-bugs` branch of this repository.
