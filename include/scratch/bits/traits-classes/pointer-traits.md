
This implementation of `pointer_traits` is significantly cleaner than a
Standard-conforming one.

The Standard definition of `pointer_traits` makes several messy assumptions
which were necessary in C++03 but have not been necessary ever since we
got `decltype` into the language. In particular, in order for
`pointer_traits<shared_ptr<T>>` to work, the Standard `pointer_traits`
has a special case for "if `Ptr` is a class template instantiation of
the form `SomePointer<T, Args>`, where `Args` is zero or more type arguments."
This allows `pointer_traits` to kinda-sorta work with smart pointer classes
even in C++03 (for example, when using `boost::shared_ptr<T>`).

In C++11 and later, however, there is no need to look for member typedef
`element_type`; the `element_type` of a pointer `p` is simply the decltype
of the expression `*p`. Similarly, the `difference_type` of a pointer is
simply the decltype of `(p - q)`.

What is a "pointer"?
--------------------

The Standard Library is full of types that a reasonable person might call
"pointer-like." Obviously there's `T*` and `shared_ptr<T>` and `unique_ptr<T>`.
There's also everything matching the `Iterator` concept — iterators are
dereferenceable and arithmeticable just like pointers. Getting silly, there's
`std::optional<T>` (it overloads `operator*`) and `T&` (it looks like a pointer
at the bits-and-bytes level but *not* at the syntactic level).

So what counts as a "pointer" for the purposes of `pointer_traits`?

The answer is that `pointer_traits` is inextricably tied up with
`allocator_traits`. The *only* purpose of `pointer_traits` is to factor
out some code from `allocator_traits` that otherwise might be duplicated
if two allocator types share a single "fancy pointer" type.

Therefore, iterators *are not pointers*. Even smart pointers like
`shared_ptr<T>` probably should not be considered "pointers" for the purposes
of `pointer_traits`, because it's hard to imagine an *allocator* that would
ever give out `shared_ptr<T>` in response to an allocation request.

An example of a "fancy pointer" type that *is* supported by real-world
allocators, and therefore must be supported by `pointer_traits`, is
[`boost::interprocess::offset_ptr<T>`](http://www.boost.org/doc/libs/1_64_0/doc/html/interprocess/offset_ptr.html).

`pointer_traits<P>::rebind<U>`
------------------------------

The one tricky case, for which special hacks are still required, is `rebind`.
The intention of `rebind` is that

- `pointer_traits<T*>::rebind<U>` should be `U*`
- `pointer_traits<shared_ptr<T>>::rebind<U>` should be `shared_ptr<U>`
- `pointer_traits<offset_ptr<T, Xs...>>::rebind<U>` should be `offset_ptr<U, Xs...>`

and so on. For this case, since `std::shared_ptr` and `std::unique_ptr`
do not provide a member template type alias `rebind`, we fall back on
the Standard's messy assumption that anything of the form
`SomePointer<T, Args>` can safely be rebound as `SomePointer<U, Args>`.

(Again, it's highly unlikely that `shared_ptr` or `unique_ptr` ought to count
as "fancy pointers." And `boost::interprocess::offset_ptr` does implement a
correct member template type alias `rebind`. So I suspect that we don't really
need to do the fallback. But I'm doing it anyway, for completeness, and to
show how it can be done.)
