#pragma once

namespace scratch {

template<class T> constexpr const T& min(const T& a, const T& b) { return (b < a) ? b : a; }
template<class T> constexpr const T& max(const T& a, const T& b) { return (b < a) ? a : b; }

template<class T, class Cmp> constexpr const T& min(const T& a, const T& b, Cmp less) { return less(b, a) ? b : a; }
template<class T, class Cmp> constexpr const T& max(const T& a, const T& b, Cmp less) { return less(b, a) ? a : b; }

} // namespace scratch
