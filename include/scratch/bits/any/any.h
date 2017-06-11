#pragma once

#include "scratch/bits/memory/unique-ptr.h"
#include "scratch/bits/stdexcept/bad-any-cast.h"
#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-fooible.h"
#include "scratch/bits/utility/in-place-type.h"

#include <initializer_list>
#include <typeinfo>
#include <utility>

namespace scratch {
    class any;  // forward declaration
} // namespace scratch

namespace scratch::detail {

template<typename T> struct is_in_place_type : false_type {};
template<typename T> struct is_in_place_type<in_place_type_t<T>> : true_type {};
template<typename T> inline constexpr bool is_in_place_type_v = is_in_place_type<T>::value;

struct any_impl_base {
    virtual void copy_to(any&) const = 0;
    virtual const std::type_info& type() const = 0;
    virtual void *get_data() const = 0;
    virtual ~any_impl_base() = default;
};

template<typename T>
struct any_impl : any_impl_base {
    void copy_to(any&) const override;
    const std::type_info& type() const override { return typeid(T); }
    void *get_data() const override { return (void *)(&m_data); }

    template<typename... Args> any_impl(Args&&... args) : m_data(std::forward<Args>(args)...) {}
private:
    T m_data;
};

} // namespace scratch::detail

namespace scratch {

class any {
    unique_ptr<detail::any_impl_base> m_ptr;
public:
    constexpr any() noexcept = default;
    any(const any& rhs) {
        if (rhs.has_value()) {
            rhs.m_ptr->copy_to(*this);
        }
    }
    any(any&& rhs) {
        m_ptr = std::move(rhs.m_ptr);
    }
    any& operator=(const any& rhs) {
        any(rhs).swap(*this);
        return *this;
    }
    any& operator=(any&& rhs) {
        any(std::move(rhs)).swap(*this);
        return *this;
    }
    ~any() = default;

    template<class T, class DT = decay_t<T>,
        class = enable_if_t<!is_same_v<DT, any> && !detail::is_in_place_type_v<DT> && is_copy_constructible_v<DT>>>
    any(T&& value) {
        this->emplace<DT>(std::forward<T>(value));
    }

    template<class DT, class... Args,
        class = enable_if_t<is_constructible_v<DT, Args...> && is_copy_constructible_v<DT>>>
    explicit any(in_place_type_t<DT>, Args&&... args) {
        this->emplace<DT>(std::forward<Args>(args)...);
    }

    template<class DT, class U, class... Args,
        class = enable_if_t<is_constructible_v<DT, std::initializer_list<U>&, Args...> && is_copy_constructible_v<DT>>>
    explicit any(in_place_type_t<DT>, std::initializer_list<U>, Args&&... args) {
        this->emplace<DT>(std::forward<Args>(args)...);
    }

    template<class T, class DT = decay_t<T>,
        class = enable_if_t<!is_same_v<DT, any> && !detail::is_in_place_type_v<DT> && is_copy_constructible_v<DT>>>
    any& operator=(T&& value) {
        any(std::forward<T>(value)).swap(*this);
        return *this;
    }

    bool has_value() const noexcept {
        return (m_ptr != nullptr);
    }

    const std::type_info& type() const noexcept {
        return m_ptr ? m_ptr->type() : typeid(void);
    }

    template<class T, class... Args>
    T& emplace(Args&&... args) {
        m_ptr = make_unique<detail::any_impl<T>>(std::forward<Args>(args)...);
        return *static_cast<T*>(m_ptr->get_data());
    }

    template<class T, class U, class... Args>
    T& emplace(std::initializer_list<U> il, Args&&... args) {
        m_ptr = make_unique<detail::any_impl<T>>(il, std::forward<Args>(args)...);
        return *static_cast<T*>(m_ptr->get_data());
    }

    void reset() noexcept {
        m_ptr = nullptr;
    }

    void swap(any& rhs) noexcept {
        m_ptr.swap(rhs.m_ptr);
    }

    template<class T> friend const T *any_cast(const any *a) noexcept;
    template<class T> friend T *any_cast(any *a) noexcept;
};

template<class T>
const T *any_cast(const any *a) noexcept {
    if (a->type() != typeid(T)) return nullptr;
    return (const T *)(a->m_ptr->get_data());
}

template<class T>
T *any_cast(any *a) noexcept {
    if (a->type() != typeid(T)) return nullptr;
    return (T *)(a->m_ptr->get_data());
}

template<class T>
T any_cast(const any& a) noexcept {
    using U = remove_cv_t<remove_reference_t<T>>;
    if (a.type() != typeid(U)) throw bad_any_cast();
    return static_cast<T>(*any_cast<U>(&a));
}

template<class T>
T any_cast(any& a) noexcept {
    using U = remove_cv_t<remove_reference_t<T>>;
    if (a.type() != typeid(U)) throw bad_any_cast();
    return static_cast<T>(*any_cast<U>(&a));
}

template<class T>
T any_cast(any&& a) noexcept {
    using U = remove_cv_t<remove_reference_t<T>>;
    if (a.type() != typeid(U)) throw bad_any_cast();
    return static_cast<T>(std::move(*any_cast<U>(&a)));
}

} // namespace scratch

namespace scratch::detail {

template<typename T>
void any_impl<T>::copy_to(any& destination) const
{
    destination.emplace<T>(m_data);
}

} // namespace scratch::detail
