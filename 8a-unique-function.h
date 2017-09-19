#pragma once

#include <functional>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

template<class Signature> struct ContainerVtable;
template<class Signature, class T> struct ContainerVtableImpl;
template<class Signature> class unique_function;

template<class R, class... A>
struct ContainerVtable<R(A...)> {
    using UF = unique_function<R(A...)>;
    void (*move_to)(UF& self, UF& dest) noexcept;   // MOVE_TO
    R (*call)(UF& self, A... args);                 // CALL
    void (*destroy)(UF& self) noexcept;             // DESTROY
};

template<class R, class... A, class T>
struct ContainerVtableImpl<R(A...), T> {
    using UF = unique_function<R(A...)>;
    using HeldType = typename UF::template held_type<T>;
    static constexpr ContainerVtable<R(A...)> vtable = {
        // move_to
        +[](UF& self, UF& dest) noexcept -> void {
            HeldType *psource = static_cast<HeldType*>(self.storage());
            ::new (dest.storage()) HeldType(std::move(*psource));
            dest.m_vptr = self.m_vptr;
            self.reset();  // I've been "moved-out-of", so reset me
        },
        // call
        +[](UF& self, A... args) noexcept -> R {
            HeldType *p = static_cast<HeldType*>(self.storage());
            if constexpr (std::is_same_v<HeldType, T>) {
                return (*p)(std::forward<A>(args)...);
            } else {
                return (**p)(std::forward<A>(args)...);
            }
        },
        // destroy
        +[](UF& self) noexcept -> void {
            static_cast<HeldType*>(self.storage())->~HeldType();
        },
    };
};

template<class R, class... A>
class unique_function<R(A...)>
{
    template<class Signature, class T> friend struct ContainerVtableImpl;

    static constexpr size_t SIZE = 64;
    static constexpr size_t ALIGN = 8;

    ContainerVtable<R(A...)> const *m_vptr = nullptr;
    std::aligned_storage_t<SIZE, ALIGN> m_buffer;

    // This is the actual type held in m_buffer: either T itself, or unique_ptr<T>.
    template<class T> using held_type = std::conditional_t<
        sizeof(T) <= SIZE && alignof(T) <= ALIGN && std::is_nothrow_move_constructible_v<T>,
        T,
        std::unique_ptr<T>
    >;

    void *storage() { return &m_buffer; }

public:
    operator bool() const noexcept {
        return (m_vptr != nullptr);
    }

    void reset() noexcept {
        if (*this) {
            m_vptr->destroy(*this);
            m_vptr = nullptr;
        }
    }

    constexpr unique_function() noexcept = default;

    template<class T, class = std::enable_if_t<!std::is_same_v<T, unique_function>>>
    unique_function(T t) {
        using HeldType = held_type<T>;
        m_vptr = &ContainerVtableImpl<R(A...), T>::vtable;
        if constexpr (std::is_same_v<HeldType, T>) {
            ::new (storage()) HeldType(std::move(t));
        } else {
            ::new (storage()) HeldType(std::make_unique<T>(std::move(t)));
        }
    }
    
    unique_function(unique_function&& rhs) noexcept {
        if (rhs) {
            rhs.m_vptr->move_to(rhs, *this);
        }
    }
    unique_function& operator=(unique_function&& rhs) noexcept {
        reset();
        if (rhs) {
            rhs.m_vptr->move_to(rhs, *this);
        }
        return *this;
    }
    unique_function& operator=(decltype(nullptr)) noexcept {
        reset();
        return *this;
    }
    ~unique_function() {
        reset();
    }

    R operator()(A... args) {
        if (*this) {
            return m_vptr->call(*this, std::forward<A>(args)...);
        } else {
            throw std::bad_function_call();
        }
    }

    void swap(unique_function& rhs) noexcept {
        unique_function temp = std::move(rhs);
        rhs = std::move(*this);
        *this = std::move(temp);
    }
};
