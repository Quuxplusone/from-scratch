The three type-traits `is_convertible`, `is_static_castable`,
and `is_constructible` are very closely related, and have the following
meanings:

- `is_convertible<A,B>`: A value of type `A` can be used in a context where
  a value of type `B` is expected. This is the same thing as
  "`A` is implicitly convertible to `B`". Generally this is true if `A`
  has a (non-`explicit`) `operator B`, or if `B` has a (non-`explicit`)
  constructor `B(A)`. Notably this is *never* true for `B=void`.

- `is_constructible<B,A>`: Type `B` has a constructor that accepts a value of
  type `A` as its parameter; in other words, `B(std::declval<A>())` is well-formed.
  Generally this is true if `A` has an `operator B` (even if it's `explicit`),
  or if `B` has a constructor `B(A)` (even if it's `explicit`). Notably
  this is *never* true for `B=void`. Also, while you might naturally expect
  `is_constructible_v<A*, void*>`, you'll find that that is *not* true,
  because `A *pa(pv)` is not a well-formed declaration.

- `is_static_castable<A,B>`: This is a non-standard type trait!
  libstdc++ actually supported a similar trait `is_explicitly_convertible<A,B>`
  for a while, although that trait was nothing more than
  `is_constructible_v<B,A> && !is_convertible_v<A,B>`.
  In our library, we've made `is_static_castable<A,B>` true if
  `static_cast<B>(declval<A>())` is well-formed. This means that
  `is_static_castable_v<A*, void*> == true`, and
  `is_static_castable_v<void*, A*> == true`(!), and also
  `is_static_castable_v<A, void> == true` for all `A`.

The non-standard type trait `is_static_castable<A,B>` is generally
the one we want when implementing (smart and/or fancy) pointer conversions:
generally we want an explicit conversion between `Ptr<A>` and `Ptr<B>` to exist
if and only if `is_static_castable_v<A*,B*>`.
