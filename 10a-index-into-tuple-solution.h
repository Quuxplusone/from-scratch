
template<size_t... Sizes>
static constexpr size_t index_into_tuple(size_t FlattenedIndex)
{
    size_t arr[] = { Sizes... };
    size_t i = 0;
    while (FlattenedIndex >= arr[i]) {
        FlattenedIndex -= arr[i];
        ++i;
    }
    return FlattenedIndex;
}

static_assert(index_into_tuple<1,3,2>(0) == 0);
static_assert(index_into_tuple<1,3,2>(1) == 0);
static_assert(index_into_tuple<1,3,2>(2) == 1);
static_assert(index_into_tuple<1,3,2>(3) == 2);
static_assert(index_into_tuple<1,3,2>(4) == 0);
static_assert(index_into_tuple<1,3,2>(5) == 1);
