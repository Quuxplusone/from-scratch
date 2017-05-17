
The following standard type traits are achievable only through compiler magic.

is_union
--------

`is_union` queries an attribute of the class that isn't exposed through any other means;
in C++, anything you can do with a class or struct, you can also do with a union. This
includes inheriting and taking member pointers.

is_aggregate, is_literal_type, is_pod, is_standard_layout, has_virtual_destructor
---------------------------------------------------------------------------------

These traits query attributes of the class that aren't exposed through any other means.
Essentially, a struct or class is a "black box"; the C++ language gives us no way to
crack it open and examine its data members to find out if they're all POD types, or if
any of them are `private`, or if the class has any constexpr constructors (the key
requirement for `is_literal_type`).

Notice that `is_polymorphic` can be detected by SFINAE on the well-formedness of
a `dynamic_cast` expression (see "is-fooible.h"), but `has_virtual_destructor` is
specifically concerned with an attribute of the destructor which is not exposed
through any other means.

is_abstract
-----------

`is_abstract` is an interesting case. The defining characteristic of an abstract
class type is that you cannot get a value of that type; so for example it is
ill-formed to define a function whose parameter or return type is abstract, and
it is ill-formed to create an array type whose element type is abstract.
(Oddly, if `T` is abstract, then SFINAE will apply to `T[]` but not to `T()`. That
is, it is acceptable to create the *type* of a function with an abstract return type;
it is ill-formed to define an *entity* of such a function type.)

So we can get very close to a correct implementation of `is_abstract` using
this SFINAE approach:

    template<class T, class> struct is_abstract_impl : true_type {};
    template<class T> struct is_abstract_impl<T, void_t<T[]>> : false_type {};

    template<class T> struct is_abstract : is_abstract_impl<remove_cv_t<T>, void> {};

However, there is a flaw! If `T` is itself a template class, such as `vector<T>`
or `basic_ostream<char>`, then merely forming the type `T[]` is acceptable; in
an unevaluated context this will *not* cause the compiler to go instantiate the
body of `T`, and therefore the compiler will not detect the ill-formedness of
the array type `T[]`. So the SFINAE will *not* happen in that case, and we'll
give the wrong answer for `is_abstract<basic_ostream<char>>`.

This quirk of template instantiation in unevaluated contexts is the sole reason
that modern compilers provide `__is_abstract(T)`.

is_literal_type
---------------

`is_literal_type` queries an attribute of the class that isn't exposed through any
other means (namely, the constexpr-ness of any one of its constructors). It turns out
that this isn't a particularly useful thing to know in any event, since the existence
of *some* constexpr constructor is no guarantee of the constexpr-ness of the constructor
that *you* want to use in whatever context you care about. Therefore, `is_literal_type`
has been [deprecated in C++17](https://stackoverflow.com/a/40352351/1424877).

The actually useful type-trait would have to be spelled something like
`is_constexpr_constructible<T, Args...>`, which suggests the ability to ask questions
about constexpr-ness in general — not just about constructibility, but about the
constexpr-ness of any arbitrary expression, just like we have `noexcept(...)` and
`decltype(...)` for arbitrary expressions in C++ today. C++ currently doesn't offer
any ability to query the constexpr-ness of expressions.

is_final
--------

`is_final` queries an attribute of the class that isn't exposed through any other means.
Specifically, the base-specifier-list of a derived class is not a SFINAE context; we can't
exploit `enable_if_t` to ask "can I create a class derived from `T`?" because if we
cannot create such a class, it'll be a hard error.

is_empty
--------

`is_empty` is an interesting case. We can't just ask whether `sizeof (T) == 0` because
in C++ no type is ever allowed to have size 0; even an empty class has `sizeof (T) == 1`.
"Emptiness" is important enough to merit a type trait, though, because of the Empty Base
Optimization: all sufficiently modern compilers will lay out the two classes

    struct Derived : public T { int x; };

    struct Underived { int x; };

identically; that is, they will not lay out any space in `Derived` for the empty
`T` subobject. This suggests a way we could test for "emptiness" in C++03, at least
on all sufficiently modern compilers: just define the two classes above and ask
whether `sizeof (Derived) == sizeof (Underived)`. Unfortunately, as of C++11, this
trick no longer works, because `T` might be final, and the "final-ness" of a class
type is not exposed by any other means! So compiler vendors who implement `final`
must also expose something like `__is_empty(T)` for the benefit of the standard library.

is_enum
-------

`is_enum` is another interesting case. Technically, we could implement this type trait
by the observation that if our type `T` is *not* a fundamental type, an array type,
a pointer type, a reference type, a member pointer, a class or union, or a function
type, then by process of elimination it must be an enum type. However, this deductive
reasoning breaks down if the compiler happens to support any other types not falling
into the above categories. For this reason, modern compilers expose `__is_enum(T)`.

A common example of a supported type not falling into any of the above categories
would be `__int128_t`. libc++ actually detects the presence of `__int128_t` and includes
it in the category of "integral types" (which makes it a "fundamental type" in the above
categorization), but our simple implementation does not.

Another example would be `vector int`, on compilers supporting Altivec vector extensions;
this type is more obviously "not integral" but also "not anything else either", and most
certainly not an enum type!

is_trivially_constructible, is_trivially_assignable
---------------------------------------------------

The triviality of construction, assignment, and destruction are all attributes of the
class that aren't exposed through any other means. Notice that with this foundation
we don't need any additional magic to query the triviality of *default* construction,
*copy* construction, *move* assignment, and so on. Instead,
`is_trivially_copy_constructible<T>` is implemented in terms of
`is_trivially_constructible<T, const T&>`, and so on.

is_trivially_destructible
-------------------------

For historical reasons, the name of this compiler builtin is not `__is_trivially_destructible(T)`
but rather `__has_trivial_destructor(T)`. Furthermore, it turns out that the builtin
evaluates to `true` even for a class type with a deleted destructor! So we first need
to check that the type is destructible; and then, if it is, we can ask the magic builtin
whether that destructor is indeed trivial. See "is-destructible.h" for the implementations
of `is_destructible` and `is_trivially_destructible`.

is_trivially_copyable
---------------------

This trait is implementable without using the compiler builtin, but its definition
is quite convoluted. According to the Standard, the type `T` is trivially copyable if

    (is_trivially_copy_constructible_v<U> || !is_copy_constructible_v<U>)
    (is_trivially_move_constructible_v<U> || !is_move_constructible_v<U>)
    (is_trivially_copy_assignable_v<U> || !is_copy_assignable_v<U>)
    (is_trivially_move_assignable_v<U> || !is_move_assignable_v<U>)
    (is_copy_constructible_v<U> || is_move_constructible_v<U> || is_copy_assignable_v<U> || is_move_assignable_v<U>)
    (is_trivially_destructible_v<U>)

where `U` represents `remove_all_extents_t<remove_const_t<T>>`.
It's simpler to just use the magic builtin.

underlying_type
---------------

The underlying type of an enum isn't exposed through any other means. You can get close
by taking `sizeof(T)` and comparing it to the sizes of all known types, and by asking
for the signedness of the underlying type via `T(-1) < T(0)`; but that approach still
cannot distinguish between underlying types `int` and `long` on platforms where those
types have the same width (nor between `long` and `long long` on platforms where *those*
types have the same width).
