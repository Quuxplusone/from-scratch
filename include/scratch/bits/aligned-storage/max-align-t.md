The typedef `std::max_align_t` must correspond to a POD type whose alignment
is "at least as strict as that of every scalar type." This is not directly
implementable in C++, because we can't get a list of every possible scalar
type — consider not only that our compiler might support `__int128_t`
but also that technically `void*` and `void****` might have different alignment
requirements.

In practice, the maximum alignment is generally the maximum alignment
of `long double`:

    using max_align_t = long double;

or at worst the maximum of the alignments of several scalar types:

    namespace detail {
        struct max_align {
            long long a;
            long double b;
            void *c;
        };
    } // namespace detail

    using max_align_t = detail::max_align;

However, defining it *that* way means that you can't use
this header on any platform where `long double` is unsupported!
(All standard-compliant C++ compilers must support `long double`, but
floating-point is often unavailable on embedded and resource-constrained
platforms. It seems awkward to require floating-point support just in order
to use `max_align_t`.)

The Standard resolves this issue by punting the problem down to C; the
`std::max_align_t` typedef is required to be defined in the `<cstddef>`
header, alongside `std::size_t` and `std::ptrdiff_t`. Since `scratch`
is already including `<cstddef>` all over the place, we simply define
`scratch::max_align_t` as a synonym for `std::max_align_t`.
