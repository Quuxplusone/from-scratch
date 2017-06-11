#pragma once

namespace scratch {

template<typename>
struct in_place_type_t {
    explicit in_place_type_t() = default;
};

template <typename T> inline constexpr in_place_type_t<T> in_place_type{};

template<typename T> struct is_in_place_type : false_type {};
template<typename T> struct is_in_place_type<in_place_type_t<T>> : true_type {};
template<typename T> inline constexpr bool is_in_place_type_v = is_in_place_type<T>::value;

} // namespace scratch
