#pragma once

#include "scratch/bits/aligned-storage/max-align-t.h"
#include "scratch/bits/containers/allocator.h"
#include "scratch/bits/stdexcept/bad-any-cast.h"
#include "scratch/bits/traits-classes/allocator-traits.h"
#include "scratch/bits/type-traits/compiler-magic.h"
#include "scratch/bits/type-traits/decay.h"
#include "scratch/bits/type-traits/enable-if.h"
#include "scratch/bits/type-traits/integral-constant.h"
#include "scratch/bits/type-traits/is-foo.h"
#include "scratch/bits/type-traits/is-fooible.h"
#include "scratch/bits/type-traits/remove-foo.h"
#include "scratch/bits/utility/in-place-type.h"

#include <new>
#include <typeinfo>
#include <utility>
#include <stdio.h>

namespace scratch {
    template<size_t Size, size_t Align, class Alloc> class basic_any;  // forward declaration
} // namespace scratch

namespace scratch::detail {

enum class basic_any_behavior {
    TYPE,
    DATA,
    COPY_TO,
    MOVE_TO,
    DESTROY,
};

template<class Derived, class Alloc, class = void>
struct basic_any_destructible {
    ~basic_any_destructible() { static_cast<Derived*>(this)->reset(); }
};

template<class Derived, class Alloc>
struct basic_any_destructible<Derived, Alloc, enable_if_t<is_inplace_allocator_v<Alloc> || is_constexpr_allocator_v<Alloc>>> {
    ~basic_any_destructible() = default;
};

template<class Any>
struct basic_any_wrapper_base {
    constexpr virtual void move_into(Any *other) = 0;
    constexpr virtual void copy_into(Any *other) const = 0;
};

template<class Any, class T>
struct basic_any_wrapper : public basic_any_wrapper_base<Any> {
    T m_data;

    template<class... Args>
    constexpr explicit basic_any_wrapper(Args&&... args) : m_data(std::forward<Args>(args)...) {}

    constexpr void move_into(Any *other) override {
        other->template emplace<T>(std::move(m_data));
    }

    constexpr void copy_into(Any *other) const override {
        other->template emplace<T>(m_data);
    }

    static constexpr const void *t_behaviors(basic_any_behavior behavior, const Any *self, Any *other) {
        switch (behavior) {
            case basic_any_behavior::TYPE: return &typeid(T);
            case basic_any_behavior::DATA: return self->template m_data<T>();
            case basic_any_behavior::COPY_TO: {
                other->reset();
                self->m_outofline->copy_into(other);
                return nullptr;
            }
            case basic_any_behavior::MOVE_TO: {
                Any *nonconst_self = const_cast<Any*>(self);
                other->reset();
                nonconst_self->m_outofline->move_into(other);
                nonconst_self->reset();
                return nullptr;
            }
            case basic_any_behavior::DESTROY: {
                Any *nonconst_self = const_cast<Any*>(self);
                using AllocT = typename Any::Alloc_traits::template rebind_alloc<T>;
                AllocT alloc(self->get_allocator());
                auto p = nonconst_self->template m_data<T>();
                allocator_traits<AllocT>::destroy(alloc, &*p);
                if constexpr (Any::has_deallocate && !Any::template fits_inline_v<T>) {
                    allocator_traits<AllocT>::deallocate(alloc, p, 1);
                }
                return nullptr;
            }
        }
        return nullptr;  // unreachable
    }
};

} // namespace scratch::detail

namespace scratch {

template<size_t Size, size_t Align = alignof(max_align_t), class Alloc = allocator<void>>
class basic_any : private detail::basic_any_destructible<basic_any<Size, Align, Alloc>, Alloc> {
    using base = detail::basic_any_destructible<basic_any<Size, Align, Alloc>, Alloc>;
    friend base;

    template<class Any, class T> friend struct detail::basic_any_wrapper;

    using allocator_type = typename allocator_traits<Alloc>::template rebind_alloc<void>;
    static constexpr bool has_allocate = !is_inplace_allocator_v<allocator_type>;
    static constexpr bool has_deallocate = !is_inplace_allocator_v<allocator_type> && !is_constexpr_allocator_v<allocator_type>;
    using Alloc_traits = allocator_traits<allocator_type>;
    template<class T> using alloc_pointer_t = typename Alloc_traits::template rebind_traits<T>::pointer;
    template<class T> static constexpr bool fits_inline_v = (sizeof(T) <= Size && alignof(T) <= Align && is_nothrow_move_constructible<T>::value);
    template<class T> static constexpr bool satisfies_requirements_v = (has_allocate || fits_inline_v<T>) && (has_deallocate || is_trivially_destructible_v<T>);

    static_assert(is_trivially_copyable_v<typename Alloc_traits::void_pointer>);

    union {
        alloc_pointer_t<detail::basic_any_wrapper_base<basic_any>> m_outofline {};
        alignas(Align ? Align : 1) char m_inline[Size ? Size : 1];
    };
    allocator_type m_alloc;
    const void *(*m_behaviors)(detail::basic_any_behavior, const basic_any*, basic_any*);

    template<class T>
    constexpr auto m_data() -> alloc_pointer_t<T> {
        if constexpr (is_void_v<T>) {
            return nullptr;
        } else if constexpr (fits_inline_v<T>) {
            return static_cast<alloc_pointer_t<T>>((T*)m_inline);
        } else {
            using Wrapper = detail::basic_any_wrapper<basic_any, T>;
            return static_cast<alloc_pointer_t<T>>(&static_cast<alloc_pointer_t<Wrapper>>(m_outofline)->m_data);
        }
    }

    template<class T>
    constexpr auto m_data() const -> alloc_pointer_t<const T> {
        if constexpr (is_void_v<T>) {
            return nullptr;
        } else if constexpr (fits_inline_v<T>) {
            return static_cast<alloc_pointer_t<const T>>((const T*)m_inline);
        } else {
            using Wrapper = detail::basic_any_wrapper<basic_any, T>;
            return static_cast<alloc_pointer_t<const T>>(&static_cast<alloc_pointer_t<Wrapper>>(m_outofline)->m_data);
        }
    }

    static constexpr const void *default_behaviors(detail::basic_any_behavior behavior, const basic_any *self [[maybe_unused]], basic_any *other) {
        switch (behavior) {
            case detail::basic_any_behavior::TYPE: return &typeid(void);
            case detail::basic_any_behavior::DATA: return nullptr;
            case detail::basic_any_behavior::COPY_TO: {
                other->reset();
                return nullptr;
            }
            case detail::basic_any_behavior::MOVE_TO: {
                other->reset();
                return nullptr;
            }
            case detail::basic_any_behavior::DESTROY: {
                return nullptr;
            }
        }
        return nullptr;  // unreachable
    }

public:
    constexpr basic_any() noexcept : m_behaviors(default_behaviors) {}
    constexpr basic_any(const basic_any& rhs) : basic_any() {
        rhs.m_behaviors(detail::basic_any_behavior::COPY_TO, &rhs, this);
    }
    constexpr basic_any(basic_any&& rhs) noexcept : basic_any() {
        rhs.m_behaviors(detail::basic_any_behavior::MOVE_TO, &rhs, this);
    }
    constexpr basic_any& operator=(const basic_any& rhs) {
        basic_any(rhs).swap(*this);
        return *this;
    }
    constexpr basic_any& operator=(basic_any&& rhs) noexcept {
        basic_any(std::move(rhs)).swap(*this);
        return *this;
    }
    constexpr void swap(basic_any& rhs) noexcept {
        basic_any temp(std::move(rhs));
        m_behaviors(detail::basic_any_behavior::MOVE_TO, this, &rhs);
        temp.m_behaviors(detail::basic_any_behavior::MOVE_TO, &temp, this);
    }
    constexpr void reset() noexcept {
        m_behaviors(detail::basic_any_behavior::DESTROY, this, nullptr);
        m_behaviors = default_behaviors;
    }


    template<class T, class DT = decay_t<T>,
        class = enable_if_t<!is_same_v<DT, basic_any> && !is_in_place_type_v<DT> && satisfies_requirements_v<DT>>>
    constexpr basic_any(T&& value) : basic_any() {
        this->emplace<DT>(std::forward<T>(value));
    }

    template<class DT, class... Args,
        class = enable_if_t<is_constructible_v<DT, Args...> && satisfies_requirements_v<DT>>>
    constexpr explicit basic_any(in_place_type_t<DT>, Args&&... args) : basic_any() {
        this->emplace<DT>(std::forward<Args>(args)...);
    }

    template<class T, class DT = decay_t<T>,
        class = enable_if_t<!is_same_v<DT, basic_any> && !is_in_place_type_v<DT> && satisfies_requirements_v<DT>>>
    constexpr basic_any& operator=(T&& value) {
        basic_any(std::forward<T>(value)).swap(*this);
        return *this;
    }

    constexpr bool has_value() const noexcept { return m_behaviors(detail::basic_any_behavior::DATA, this, nullptr) != nullptr; }
    constexpr const void *get_data() const noexcept { return m_behaviors(detail::basic_any_behavior::DATA, this, nullptr); }
    constexpr void *get_data() noexcept { return const_cast<void*>(m_behaviors(detail::basic_any_behavior::DATA, this, nullptr)); }
    constexpr allocator_type get_allocator() const noexcept { return m_alloc; }

    constexpr const std::type_info& type() const noexcept {
        return *static_cast<const std::type_info *>(m_behaviors(detail::basic_any_behavior::TYPE, this, nullptr));
    }

private:
    template<class T, class... Args>
    inline void allocate_and_construct(Args&&... args) {
        using Wrapper = detail::basic_any_wrapper<basic_any, T>;
        using AllocWrapper = typename Alloc_traits::template rebind_alloc<Wrapper>;
        AllocWrapper alloc(get_allocator());
        auto fancy_ptr = allocator_traits<AllocWrapper>::allocate(alloc, 1);
        Wrapper *raw_ptr = &*fancy_ptr;
        try {
            allocator_traits<AllocWrapper>::construct(alloc, raw_ptr, std::forward<Args>(args)...);
        } catch (...) {
            allocator_traits<AllocWrapper>::deallocate(alloc, fancy_ptr, 1);
            throw;
        }
        m_outofline = std::move(fancy_ptr);
    }

public:
    template<class T, class... Args>
    constexpr T& emplace(Args&&... args) {
        reset();
        if constexpr (fits_inline_v<T>) {
            T *raw_ptr = &*m_data<T>();
            ::new ((void*)raw_ptr) T(std::forward<Args>(args)...);
        } else if constexpr (!has_deallocate) {
            using Wrapper = detail::basic_any_wrapper<basic_any, T>;
            using AllocWrapper = typename Alloc_traits::template rebind_alloc<Wrapper>;
            AllocWrapper alloc(get_allocator());
            auto fancy_ptr = allocator_traits<AllocWrapper>::allocate(alloc, 1);
            Wrapper *raw_ptr = &*fancy_ptr;
            allocator_traits<AllocWrapper>::construct(alloc, raw_ptr, std::forward<Args>(args)...);
            m_outofline = std::move(fancy_ptr);
        } else {
            allocate_and_construct<T>(std::forward<Args>(args)...);
        }
        m_behaviors = detail::basic_any_wrapper<basic_any, T>::t_behaviors;
        return *m_data<T>();
    }

    template<class T, size_t A, size_t B, class C> friend constexpr const T *any_cast(const basic_any<A,B,C> *a) noexcept;
    template<class T, size_t A, size_t B, class C> friend constexpr T *any_cast(basic_any<A,B,C> *a) noexcept;
};

template<size_t A, size_t B, class C>
constexpr void swap(basic_any<A,B,C>& a, basic_any<A,B,C>& b) noexcept {
    a.swap(b);
}

template<class T, size_t A, size_t B, class C>
constexpr const T *any_cast(const basic_any<A,B,C> *a) noexcept {
    if (a->type() != typeid(T)) return nullptr;
    return (const T *)(a->get_data());
}

template<class T, size_t A, size_t B, class C>
constexpr T *any_cast(basic_any<A,B,C> *a) noexcept {
    if (a->type() != typeid(T)) return nullptr;
    return (T *)(a->get_data());
}

template<class T, size_t A, size_t B, class C>
constexpr T any_cast(const basic_any<A,B,C>& a) {
    using U = remove_cv_t<remove_reference_t<T>>;
    if (a.type() != typeid(U)) throw bad_any_cast();
    return static_cast<T>(*any_cast<U>(&a));
}

template<class T, size_t A, size_t B, class C>
constexpr T any_cast(basic_any<A,B,C>& a) {
    using U = remove_cv_t<remove_reference_t<T>>;
    if (a.type() != typeid(U)) throw bad_any_cast();
    return static_cast<T>(*any_cast<U>(&a));
}

template<class T, size_t A, size_t B, class C>
constexpr T any_cast(basic_any<A,B,C>&& a) {
    using U = remove_cv_t<remove_reference_t<T>>;
    if (a.type() != typeid(U)) throw bad_any_cast();
    return static_cast<T>(std::move(*any_cast<U>(&a)));
}

} // namespace scratch
