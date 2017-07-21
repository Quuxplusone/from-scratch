The exception type `scratch::bad_alloc` ought to be a direct descendant of
`scratch::exception`. However, it turns out to be pretty ugly to implement
`bad_alloc` in "user space", because it is one of those types (like
`std::type_info` and `std::initializer_list<T>`) whose implementation is
fundamentally part of the C++ runtime library.

The global runtime function `::operator new`, which is called by expressions
such as `new char[10]`, throws `std::bad_alloc` on failure. This means that there's
actual compiled code in the runtime library that knows how to construct an
object of type `bad_alloc` and throw it. We cannot simulate this.

What we *could* do is provide our own copy of the standard header `<new>`,
where we override the definitions of `::operator new` and `::operator delete`
for all 22 of the signatures currently listed on
[cppreference](http://en.cppreference.com/w/cpp/memory/new/operator_new).
This currently seems to me like overkill.

The disadvantage of my current approach is that `scratch::bad_alloc` ends
up not inheriting from `scratch::exception` at all, which would cause
major problems for idiomatic C++ code that actually tried to use this
library. (I mean, if you wrote `catch (scratch::exception&)`, it would
catch every standard exception *except* `bad_alloc`. This would be bad.)
But nobody should use this library for production code anyway, so I think
we're okay for now.
