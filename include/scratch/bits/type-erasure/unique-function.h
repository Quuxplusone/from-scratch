#pragma once

#include "scratch/bits/aligned-storage/aligned-storage.h"
#include "scratch/bits/stdexcept/bad-function-call.h"
#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-fooible.h"

#include <new>
#include <typeinfo>
#include <utility>

namespace scratch {
    template<class Signature> class unique_function;  // forward declaration
} // namespace scratch

namespace scratch::detail {

enum class unique_function_impl_behavior_e {
    TYPE,
    DATA,
    MOVE_TO,
    DESTROY,
    TRAMPOLINE,
};

union unique_function_impl_storage {
private:
    aligned_storage_t<16, 8> m_inline = {};
    void *m_large;

    template<class T>
    static inline constexpr bool fits_inline =
        sizeof(T) <= sizeof(m_inline) && alignof(T) <= alignof(decltype(m_inline)) && is_nothrow_move_constructible_v<T>;

public:
    constexpr unique_function_impl_storage() noexcept = default;

    template<class Signature> struct behaviors;  // forward declaration

    template<class R, class... A>
    struct behaviors<R(A...)> {
        template<class T>
        static void of(unique_function_impl_behavior_e what, unique_function<R(A...)>& who, void *p);
    };

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

template<class Signature> class unique_function;  // forward declaration

template<class R, class... A>
class unique_function<R(A...)> {
    friend union detail::unique_function_impl_storage;

    void (*m_behaviors)(detail::unique_function_impl_behavior_e, unique_function&, void *) = nullptr;
    detail::unique_function_impl_storage m_storage;

    auto get_trampoline() const {
        using PlainFunction = R(void *, A&&...);
        PlainFunction *result = nullptr;
        if (m_behaviors) {
            m_behaviors(detail::unique_function_impl_behavior_e::TRAMPOLINE, const_cast<unique_function&>(*this), &result);
        }
        return result;
    }

    void *get_data() const {
        void *result = nullptr;
        if (m_behaviors) {
            m_behaviors(detail::unique_function_impl_behavior_e::DATA, const_cast<unique_function&>(*this), &result);
        }
        return result;
    }

    template<class T, class... Args>
    T& emplace(Args&&... args) {
        reset();
        m_storage.emplace<T>(std::forward<Args>(args)...);
        m_behaviors = detail::unique_function_impl_storage::behaviors<R(A...)>::template of<T>;
        return *static_cast<T*>(get_data());
    }

    void reset() noexcept {
        if (m_behaviors) {
            m_behaviors(detail::unique_function_impl_behavior_e::DESTROY, *this, nullptr);
        }
        m_behaviors = nullptr;
    }

public:
    constexpr unique_function() noexcept = default;
    unique_function(unique_function&& rhs) noexcept {
        if (rhs) {
            rhs.m_behaviors(detail::unique_function_impl_behavior_e::MOVE_TO, rhs, this);
        }
    }
    unique_function& operator=(unique_function&& rhs) noexcept {
        if (rhs) {
            rhs.m_behaviors(detail::unique_function_impl_behavior_e::MOVE_TO, rhs, this);
        } else {
            reset();
        }
        return *this;
    }
    ~unique_function() = default;

    template<class T, class DT = decay_t<T>,
        class = enable_if_t<!is_same_v<DT, unique_function> && is_move_constructible_v<DT>>>
    unique_function(T&& value) {
        this->emplace<DT>(std::forward<T>(value));
    }

    operator bool() const noexcept {
        return (m_behaviors != nullptr);
    }

    const std::type_info& target_type() const noexcept {
        if (*this) {
            const std::type_info *result = nullptr;
            m_behaviors(detail::unique_function_impl_behavior_e::TYPE, const_cast<unique_function&>(*this), &result);
            return *result;
        } else {
            return typeid(void);
        }
    }

    R operator()(A... args) {
        if (*this) {
            auto trampoline = this->get_trampoline();
            void *p = this->get_data();
            return trampoline(p, std::forward<A>(args)...);
        } else {
            throw bad_function_call();
        }
    }

    template<class F, class = enable_if_t<!is_same_v<decay_t<F>, unique_function>>>
    unique_function& operator=(F f) {
        unique_function(std::move(f)).swap(*this);
        return *this;
    }

    void swap(unique_function& rhs) noexcept {
        unique_function temp = std::move(rhs);
        rhs = std::move(*this);
        *this = std::move(temp);
    }
};

} // namespace scratch

namespace scratch::detail {

template<class R, class... A>
template<class T>
void unique_function_impl_storage::behaviors<R(A...)>::of(
    unique_function_impl_behavior_e what,
    unique_function<R(A...)>& who,
    void *p)
{
    void *data = unique_function_impl_storage::fits_inline<T> ? &who.m_storage.m_inline : who.m_storage.m_large;
    switch (what) {
        case unique_function_impl_behavior_e::TYPE:
            *(const std::type_info**)p = &typeid(T);
            break;
        case unique_function_impl_behavior_e::DATA:
            *(void**)p = data;
            break;
        case unique_function_impl_behavior_e::MOVE_TO:
            ((unique_function<R(A...)>*)p)->template emplace<T>(std::move(*(T*)data));
            who.reset();
            break;
        case unique_function_impl_behavior_e::DESTROY:
            if constexpr (unique_function_impl_storage::fits_inline<T>) {
                ((T *)data)->~T();
            } else {
                delete (T*)who.m_storage.m_large;
            }
            break;
        case unique_function_impl_behavior_e::TRAMPOLINE:
            using PlainFunction = R(void *, A&&...);
            auto trampoline = [](void *t, A&&... args) -> R { return (*(T*)t)(std::forward<A>(args)...); };
            *(PlainFunction **)p = trampoline;
            break;
    }
}

} // namespace scratch::detail
