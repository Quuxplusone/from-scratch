
is_function
-----------

This implementation of `is_function<T>` is due to Alisdair Meredith, with
improvements by Johannes Schaub. It works by the clever observation that
function types and reference types are the only C++ types which are
unaffected by the `const` qualifier. Therefore, if `!is_const_v<const T>`
and `T` is not a reference type, then `T` must be a function type.

is_abominable_function, is_non_abominable_function
--------------------------------------------------

All the C++ function types you'll see in practice look like this: `Ret(Args...)`.
However, this accounts only for function types with a defined number of parameters;
it doesn't account for C-style variadic functions such as `printf`. Variadic
function types look like this: `Ret(Args..., ...)`. These two kinds of function
types account for all the function types that can actually belong to C++ functions.

However, there is another family of function types in C++! The so-called
"abominable function types" are the types that you get by taking a
pointer-to-qualified-member-function such as `void (MyClass::*)(int) const`
and removing the pointer: `void(int) const`. For each normal function type
(for example, `void(int)`) there are 11 abominable function types:

    void(int) const
    void(int) volatile
    void(int) const volatile
    void(int) &
    void(int) const &
    void(int) volatile &
    void(int) const volatile &
    void(int) &&
    void(int) const &&
    void(int) volatile &&
    void(int) const volatile &&

The Standard does not supply a type trait for "abominable" function types,
but `add_pointer<T>` needs to detect when `T` is an abominable function
type and refrain from adding `*` in that case.

is_referenceable
----------------

The Standard defines "referenceable type" as "an object type, a function type
that does not have cv-qualifiers or a ref-qualifier [that is to say, a
non-abominable function type], or a reference type." There is no Standard
type trait corresponding to this definition.
