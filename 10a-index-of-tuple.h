
template<size_t... Sizes>
static constexpr size_t index_of_tuple(size_t FlattenedIndex)
{

    // YOUR CODE GOES HERE

}

static_assert(index_of_tuple<1,3,2>(0) == 0);
static_assert(index_of_tuple<1,3,2>(1) == 1);
static_assert(index_of_tuple<1,3,2>(2) == 1);
static_assert(index_of_tuple<1,3,2>(3) == 1);
static_assert(index_of_tuple<1,3,2>(4) == 2);
static_assert(index_of_tuple<1,3,2>(5) == 2);
