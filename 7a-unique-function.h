#pragma once

#include <functional>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace my {

template<class Signature, class T> struct ContainerVtableImpl;
template<class Signature> class unique_function;

enum class VtableIndex { MOVE_TO, CALL, DESTROY };

template<class R, class... A, class T>
struct ContainerVtableImpl<R(A...), T> {
    using UF = unique_function<R(A...)>;

    static auto vtable_in_code(UF& self, VtableIndex vtable_index, UF *dest) -> typename UF::TrampolineType*
    {
        using HeldType = typename UF::template held_type<T>;
        switch (vtable_index) {
            case VtableIndex::MOVE_TO: {
                HeldType *psource = static_cast<HeldType*>(self.storage());
                ::new (dest->storage()) HeldType(std::move(*psource));
                dest->m_behaviors = self.m_behaviors;
                self.reset();  // I've been "moved-out-of", so reset me
                return nullptr;
            }
            case VtableIndex::CALL: {
                // In order to actually make the call in here, I'd need to pass in the arguments
                // somehow, and they're of some possibly-awkward types "A...". So instead, I'll
                // keep this one in "pointer to function" form: I'll return a pointer to a function
                // that my caller can call.
                return [](UF& self, A... args) {
                    HeldType *p = static_cast<HeldType*>(self.storage());
                    if constexpr (std::is_same_v<HeldType, T>) {
                        return (*p)(std::forward<A>(args)...);
                    } else {
                        return (**p)(std::forward<A>(args)...);
                    }
                };
            }
            case VtableIndex::DESTROY: {
                static_cast<HeldType*>(self.storage())->~HeldType();
                return nullptr;
            }
        }
        __builtin_unreachable();
    }
    
    // Sanity-check that our "vtable_in_code" function is exactly the type expected by
    // our corresponding unique_function's "m_behaviors" function pointer.
    static_assert(std::is_same_v<decltype(&vtable_in_code), typename UF::VtableInCodeType*>);
};

template<class R, class... A>
class unique_function<R(A...)>
{
    template<class Signature, class T> friend struct ContainerVtableImpl;

    static constexpr size_t SIZE = 64;
    static constexpr size_t ALIGN = 8;

    using UF = unique_function<R(A...)>;
    using TrampolineType = R(UF&, A...);  // see the implementation of operator()(A...)
    using VtableInCodeType = TrampolineType *(UF&, VtableIndex, UF*);  // this is a function type

    VtableInCodeType *m_behaviors = nullptr;  // this is a function pointer
    std::aligned_storage_t<SIZE, ALIGN> m_buffer;

    // held_type<T> is the actual type held in m_buffer: either T itself, or unique_ptr<T>.
    template<class T> using held_type = std::conditional_t<
        sizeof(T) <= SIZE && alignof(T) <= ALIGN && std::is_nothrow_move_constructible_v<T>,
        T,
        std::unique_ptr<T>
    >;

    void *storage() { return &m_buffer; }

public:
    operator bool() const noexcept {
        return (m_behaviors != nullptr);
    }

    void reset() noexcept {
        if (*this) {
            m_behaviors(*this, VtableIndex::DESTROY, nullptr);
            m_behaviors = nullptr;
        }
    }

    constexpr unique_function() noexcept = default;

    template<class T, class = std::enable_if_t<!std::is_same_v<T, unique_function>>>
    unique_function(T t) {
        using HeldType = held_type<T>;
        m_behaviors = &ContainerVtableImpl<R(A...), T>::vtable_in_code;
        if constexpr (std::is_same_v<HeldType, T>) {
            ::new (storage()) HeldType(std::move(t));
        } else {
            ::new (storage()) HeldType(std::make_unique<T>(std::move(t)));
        }
    }
    
    unique_function(unique_function&& rhs) noexcept {
        if (rhs) {
            rhs.m_behaviors(rhs, VtableIndex::MOVE_TO, this);
        }
    }
    unique_function& operator=(unique_function&& rhs) noexcept {
        reset();
        if (rhs) {
            rhs.m_behaviors(rhs, VtableIndex::MOVE_TO, this);
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
            // Retrieve a function pointer---
            TrampolineType *call = m_behaviors(*this, VtableIndex::CALL, nullptr);
            // ---and then call through it.
            return (*call)(*this, std::forward<A>(args)...);
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

} // namespace my
