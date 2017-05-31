
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
`shared_ptr<T>` should not be considered "pointers" for the purposes
of `pointer_traits`, because it's hard to imagine an *allocator* that would
ever give out `shared_ptr<T>` in response to an allocation request.

An example of a "fancy pointer" type that *is* supported by real-world
allocators, and therefore must be supported by `pointer_traits`, is
[`boost::interprocess::offset_ptr<T>`](http://www.boost.org/doc/libs/1_64_0/doc/html/interprocess/offset_ptr.html).

What is `rebind`?
-----------------

The alias template `pointer_traits<P>::rebind<U>` and the
alias template `allocator_traits<A>::rebind_alloc<U>`
both have the same purpose and the same essential motivation:
untangling the mess that started when the Standard decided
that the syntax for allocators should be

    std::list<int, std::allocator<int>> lst;

See, `std::list` gets an allocator that knows how to allocate `int`s; but
`std::list` doesn't *want* to allocate `int`s! It wants to allocate some awkwardly
sized *node* struct, something like `detail::list_node<int, typename std::allocator<int>::pointer>`
— let's just call it `Node` for short. So the library needs some way to take an
`allocator<int>` and turn it into an `allocator<Node>`. This is what `rebind`
does. `std::allocator_traits<std::allocator<int>>::rebind_alloc<Node>`
evaluates to `std::allocator<Node>`.

Likewise, if we've got a `fancy_pointer<Node>` and we want to convert it to
its `const`-pointee-qualified version, we'll use
`pointer_traits<fancy_pointer<Node>>::rebind<const Node>`, which evaluates to
`fancy_pointer<const Node>`.

If your allocator or fancy-pointer type defines a nested alias template `rebind`,
then `allocator_traits` resp. `pointer_traits` will pick it up. But generally
speaking you don't need to worry about `rebind`, as long as your fancy pointer
type has its `element_type` as its first template parameter; see below.

(Also, the name of the allocator's nested alias template has to be named
`rebind<U>::other`, not just `rebind<U>`, because allocators come from
C++03 (which didn't have alias templates) whereas fancy pointers come from
C++11 (which did).

So what does my fancy pointer type actually need to implement?
--------------------------------------------------------------

The Standard `pointer_traits` is extremely convoluted in its requirements
and defaults.

Q: Why does my fancy pointer type need to provide a nested typedef `difference_type`?
Shouldn't `decltype(p - p)` be good enough?

A: `decltype(p - p)` fails to work for fancy pointers to `void`, since void pointers
generally can't be subtracted. But if you don't provide `difference_type`, the trait
will default to `ptrdiff_t`, which is invariably correct.

Q: Why does my fancy pointer type need to provide a nested typedef `element_type`?
Shouldn't `decltype(*p)` be good enough?

A: Again, `decltype(*p)` fails to work for fancy pointers to `void`.

Q: Do I *actually* need to provide `element_type`, though?

A: Well, not usually. The Standard `pointer_traits` actually has a special case for
any fancy-pointer type that looks like `Foo<Bar, Baz...>`: it will assume that if
you're messing with `pointer_traits` for such a type, then a good default behavior
would be to assume that `element_type` is `Bar` and that `rebind<U>` is
`Foo<U, Baz...>`. This is completely crazy from a philosophical point of view
(the core language doesn't assign any magical meaning to the first template parameter!),
but it does work very conveniently in practice. It means that generally you don't
have to define `element_type` or `rebind` — and of course you basically never have
to define `difference_type`, because `ptrdiff_t` is always an okay choice.

Q: Why does my fancy pointer type need to provide nested typedefs `value_type`,
`reference`, `pointer`, `difference_type`, and `iterator_category`?

A: Because every fancy pointer type (if it's going to be typedeffed as `A::pointer`
for any allocator `A`) must also be a random-access iterator type, as required by
[allocator.requirements]/5. And that means that we need `iterator_traits<P>` to be
populated; but `iterator_traits<P>` is populated only for types that provide all
five of those nested typedefs. So you have to provide them all.

Ironically, this means that a type that wants to be a "fancy pointer" needs to
spend more time worrying about *iterator* traits than about *pointer* traits!

Clang's libc++ containers actually go out of their way to do iteration
(e.g. `std::copy`) only on raw `T*`, never on fancy pointers. But GNU's libstdc++
does iteration on fancy pointers all the time; e.g. in `vector::resize`. So if
you want your fancy pointers to work on libstdc++, you'd better make sure they're
safe for use as iterators.
