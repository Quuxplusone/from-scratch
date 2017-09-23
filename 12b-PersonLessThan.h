
// Your task, should you choose to accept it, is to turn this comparator class
// into a "transparent" comparator.
// Reading http://en.cppreference.com/w/cpp/container/set/count should help.

struct PersonLessThan
{
    bool operator()(const my::Person& a, const my::Person& b) const {
        return a.salient_bits() < b.salient_bits();
    }
#if 0
    bool operator()(const my::Person& a, std::string_view b) const {
        return a.to_string() < b;
    }
    // YOUR CODE GOES HERE
    using is_transparent = // YOUR CODE GOES HERE
#endif // 0
};
