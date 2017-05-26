
`iterator_traits<It>` stores information about iterator types. In C++17,
with `decltype` at our fingertips, this traits class isn't as useful as
it once was. Its primary purpose is to store `iterator_category`, which
tells the standard algorithms whether a given iterator is forward-only,
bidirectional, or random-access.

Notice that even though `ContiguousIterator` is a concept blessed by
the C++17 standard, there is no iterator-category tag for contiguous
iterators.

I've introduced non-standard convenience aliases for the two fundamentally
useful member typedefs:

- Instead of `typename iterator_traits<T>::difference_type`, you can write simply `iterator_difference_t<T>`.
- Instead of `typename iterator_traits<T>::iterator_category`, you can write simply `iterator_category_t<T>`.

In addition, in the header "is-foo-iterator.h", I've provided these
convenience variables:

- Instead of `is_convertible_v<iterator_category_t<T>, input_iterator_tag>`,
  you can write simply `is_input_iterator_v<T>`.
- Instead of `is_convertible_v<iterator_category_t<T>, output_iterator_tag>`,
  you can write simply `is_output_iterator_v<T>`.
- Instead of `is_convertible_v<iterator_category_t<T>, forward_iterator_tag>`,
  you can write simply `is_forward_iterator_v<T>`.
- Instead of `is_convertible_v<iterator_category_t<T>, bidirectional_iterator_tag>`,
  you can write simply `is_bidirectional_iterator_v<T>`.
- Instead of `is_convertible_v<iterator_category_t<T>, random_access_iterator_tag>`,
  you can write simply `is_random_access_iterator_v<T>`.

The member typedefs `pointer`, `reference`, and `value_type` are not only
rarely useful in our post-`decltype` landscape; they're often actively
incorrect. For example, `std::back_insert_iterator` defines all three
member typedefs as `void`. Just using `decltype(*it)` or whatever will
be a lot easier than wrangling these member typedefs.
