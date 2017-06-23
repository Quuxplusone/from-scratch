#pragma once

#include "scratch/bits/smart-ptrs/shared-ptr.h"
#include "scratch/bits/smart-ptrs/weak-ptr.h"
#include "scratch/bits/stdexcept/bad-weak-ptr.h"

namespace scratch {

template<typename T>
class enable_shared_from_this {
    mutable weak_ptr<T> m_weak;
public:
    constexpr enable_shared_from_this() noexcept = default;
    enable_shared_from_this(const enable_shared_from_this<T>&) noexcept {}
    enable_shared_from_this<T>& operator=(const enable_shared_from_this<T>&) noexcept { return *this; }

    // This method is NOT part of the standard enable_shared_from_this!
    void enable_sharing_from_this(const shared_ptr<Y>& sptr) const {
        if (m_weak.expired()) {
            m_weak = shared_ptr<T>(sptr);
        }
    }

    weak_ptr<T> weak_from_this() { return m_weak; }
    weak_ptr<const T> weak_from_this() const { return m_weak; }

    // These methods throw bad_weak_ptr on error.
    shared_ptr<T> shared_from_this() { return shared_ptr<T>(m_weak); }
    shared_ptr<const T> shared_from_this() const { return shared_ptr<T>(m_weak); }
};

} // namespace scratch
