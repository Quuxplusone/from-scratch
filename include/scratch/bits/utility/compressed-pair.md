
`compressed_pair` is a utility type that will be used extensively within our implementations
of standard library types, but is not itself a standard library type.

The structure of this `compressed_pair` implementation is based on the one from libc++.
A `compressed_pair<A,B>` is simply a conglomeration of a `compressed_element<A,0>` and
a `compressed_element<B,1>`, which exploit the Empty Base Optimization where possible.

Notice that `compressed_pair` cannot possibly have `first` and `second` member variables,
because it may not have "member variables" at all — just base classes!  We could give
it `first()` and `second()` member functions instead, but it's simpler to just use
`scratch::get<0>()` and `scratch::get<1>()`.

Once we have `compressed_element`, we can equally well express `tuple<Ts...>` as a
conglomeration of `compressed_element<Ts,Is>...`.
