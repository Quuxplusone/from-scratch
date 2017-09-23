// You do not need to edit this file.

#include <sstream>
#include <string>
#include <tuple>

namespace my {

struct Person {
    std::string first_name;
    std::string last_name;
    int age;
    
    explicit Person(std::string first, std::string last, int age) :
        first_name(first), last_name(last), age(age) {}
    // Do not add any new constructors to this class.

    auto salient_bits() const { return std::tie(first_name, last_name); }

    bool operator==(const Person& rhs) const { return salient_bits() == rhs.salient_bits(); }
    bool operator!=(const Person& rhs) const { return salient_bits() != rhs.salient_bits(); }
    bool operator< (const Person& rhs) const { return salient_bits() <  rhs.salient_bits(); }
    bool operator<=(const Person& rhs) const { return salient_bits() <= rhs.salient_bits(); }
    bool operator> (const Person& rhs) const { return salient_bits() >  rhs.salient_bits(); }
    bool operator>=(const Person& rhs) const { return salient_bits() >= rhs.salient_bits(); }
    
    friend auto& operator<<(std::ostream& os, const Person& p) {
        return (os << p.first_name << ' ' << p.last_name);
    }
    auto to_string() const { std::ostringstream oss; oss << *this; return oss.str(); }
};

} // namespace my

// You do not need to edit this file.
