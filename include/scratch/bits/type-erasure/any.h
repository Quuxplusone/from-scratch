#pragma once

#include "scratch/bits/aligned-storage/aligned-storage.h"
#include "scratch/bits/stdexcept/bad-any-cast.h"
#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-fooible.h"
#include "scratch/bits/type-traits/remove-foo.h"
#include "scratch/bits/utility/in-place-type.h"

#include <initializer_list>
#include <new>
#include <typeinfo>
#include <utility>

namespace scratch {
    class any;  // forward declaration
} // namespace scratch

namespace scratch::detail {

template<typename T> struct is_in_place_type : false_type {};
template<typename T> struct is_in_place_type<in_place_type_t<T>> : true_type {};
template<typename T> inline constexpr bool is_in_place_type_v = is_in_place_type<T>::value;

enum class any_impl_behavior_e {
    TYPE,
    DATA,
    COPY_TO,
    MOVE_TO,
    DESTROY,
};

union any_impl_storage {
private:
    aligned_storage_t<16, 8> m_inline = {};
    void *m_large;

    template<class T>
    static inline constexpr bool fits_inline =
        sizeof(T) <= sizeof(m_inline) && alignof(T) <= alignof(decltype(m_inline)) && is_nothrow_move_constructible_v<T>;

public:
    constexpr any_impl_storage() noexcept = default;

    template<class T>
    static void behaviors(any_impl_behavior_e what, any& who, void *p);

    template<class T, class... Args>
    void emplace(Args&&... args) {
        if constexpr (fits_inline<T>) {
            ::new ((void*)&m_inline) T(std::forward<Args>(args)...);
        } else {
            m_large = new T(std::forward<Args>(args)...);
        }
    }
};

} // namespace scratch::detail

namespace scratch {

class any {
    friend union detail::any_impl_storage;

    void (*m_behaviors)(detail::any_impl_behavior_e, any&, void *) = nullptr;
    detail::any_impl_storage m_storage;

    void *get_data() const {
        void *result = nullptr;
        if (has_value()) {
            m_behaviors(detail::any_impl_behavior_e::DATA, const_cast<any&>(*this), &result);
        }
        return result;
    }

public:
    constexpr any() noexcept = default;
    any(const any& rhs) {
        if (rhs.has_value()) {
            rhs.m_behaviors(detail::any_impl_behavior_e::COPY_TO, const_cast<any&>(rhs), this);
        }
    }
    any(any&& rhs) noexcept {
        if (rhs.has_value()) {
            rhs.m_behaviors(detail::any_impl_behavior_e::MOVE_TO, rhs, this);
        }
    }
    any& operator=(const any& rhs) {
        any(rhs).swap(*this);
        return *this;
    }
    any& operator=(any&& rhs) noexcept {
        if (rhs.has_value()) {
            rhs.m_behaviors(detail::any_impl_behavior_e::MOVE_TO, rhs, this);
        } else {
            reset();
        }
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
        return (m_behaviors != nullptr);
    }

    const std::type_info& type() const noexcept {
        if (has_value()) {
            const std::type_info *result = nullptr;
            m_behaviors(detail::any_impl_behavior_e::TYPE, const_cast<any&>(*this), &result);
            return *result;
        } else {
            return typeid(void);
        }
    }

    template<class T, class... Args>
    T& emplace(Args&&... args) {
        reset();
        m_storage.emplace<T>(std::forward<Args>(args)...);
        m_behaviors = detail::any_impl_storage::behaviors<T>;
        return *static_cast<T*>(get_data());
    }

    template<class T, class U, class... Args>
    T& emplace(std::initializer_list<U> il, Args&&... args) {
        reset();
        m_storage.emplace<T>(il, std::forward<Args>(args)...);
        m_behaviors = detail::any_impl_storage::behaviors<T>;
        return *static_cast<T*>(get_data());
    }

    void reset() noexcept {
        if (has_value()) {
            m_behaviors(detail::any_impl_behavior_e::DESTROY, *this, nullptr);
        }
        m_behaviors = nullptr;
    }

    void swap(any& rhs) noexcept {
        any temp = std::move(rhs);
        rhs = std::move(*this);
        *this = std::move(temp);
    }

    template<class T> friend const T *any_cast(const any *a) noexcept;
    template<class T> friend T *any_cast(any *a) noexcept;
};

template<class T>
const T *any_cast(const any *a) noexcept {
    if (a->type() != typeid(T)) return nullptr;
    return (const T *)(a->get_data());
}

template<class T>
T *any_cast(any *a) noexcept {
    if (a->type() != typeid(T)) return nullptr;
    return (T *)(a->get_data());
}

template<class T>
T any_cast(const any& a) {
    using U = remove_cv_t<remove_reference_t<T>>;
    if (a.type() != typeid(U)) throw bad_any_cast();
    return static_cast<T>(*any_cast<U>(&a));
}

template<class T>
T any_cast(any& a) {
    using U = remove_cv_t<remove_reference_t<T>>;
    if (a.type() != typeid(U)) throw bad_any_cast();
    return static_cast<T>(*any_cast<U>(&a));
}

template<class T>
T any_cast(any&& a) {
    using U = remove_cv_t<remove_reference_t<T>>;
    if (a.type() != typeid(U)) throw bad_any_cast();
    return static_cast<T>(std::move(*any_cast<U>(&a)));
}

} // namespace scratch

namespace scratch::detail {

template<class T>
void any_impl_storage::behaviors(any_impl_behavior_e what, any& who, void *p)
{
    void *data = fits_inline<T> ? &who.m_storage.m_inline : who.m_storage.m_large;
    switch (what) {
        case any_impl_behavior_e::TYPE:
            *(const std::type_info**)p = &typeid(T);
            break;
        case any_impl_behavior_e::DATA:
            *(void**)p = data;
            break;
        case any_impl_behavior_e::COPY_TO:
            ((any*)p)->emplace<T>(*(T*)data);
            break;
        case any_impl_behavior_e::MOVE_TO:
            ((any*)p)->emplace<T>(std::move(*(T*)data));
            who.reset();
            break;
        case any_impl_behavior_e::DESTROY:
            if constexpr (fits_inline<T>) {
                ((T *)data)->~T();
            } else {
                delete (T*)who.m_storage.m_large;
            }
            break;
    }
}

} // namespace scratch::detail
