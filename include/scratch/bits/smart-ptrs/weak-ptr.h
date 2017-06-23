#pragma once

#include "scratch/bits/smart-ptrs/shared-ptr-control-block.h"
#include "scratch/bits/smart-ptrs/shared-ptr.h"
#include "scratch/bits/smart-ptrs/smart-ptr-base.h"

#include <utility>

namespace scratch {

template<typename T>
class weak_ptr : private detail::smart_ptr_base<T>
{
public:
    using element_type = remove_extent_t<T>;

    constexpr weak_ptr() noexcept = default;

    // This aliasing constructor is non-standard.
    template<class Y>
    weak_ptr(const shared_ptr<Y>& rhs, T* ptr) noexcept : m_ctrl(rhs.m_ctrl) {
        m_ptr = ptr;
        increment_weak_count();
    }

    weak_ptr(const weak_ptr& rhs) noexcept : m_ctrl(rhs.m_ctrl) {
        m_ptr = rhs.m_ptr;
        increment_weak_count();
    }

    template<class Y> weak_ptr(const weak_ptr<Y>& rhs) noexcept : m_ctrl(rhs.m_ctrl) {
        m_ptr = rhs.m_ptr;
        increment_weak_count();
    }

    weak_ptr(weak_ptr&& rhs) noexcept {
        m_ptr = std::exchange(rhs.m_ptr, nullptr);
        m_ctrl = std::exchange(rhs.m_ctrl, nullptr);
    }

    template<class Y> weak_ptr(weak_ptr<Y>&& rhs) noexcept {
        m_ptr = std::exchange(rhs.m_ptr, nullptr);
        m_ctrl = std::exchange(rhs.m_ctrl, nullptr);
    }

                      weak_ptr& operator=(const weak_ptr& rhs) noexcept    { weak_ptr<T>(rhs).swap(*this); return *this; }
    template<class Y> weak_ptr& operator=(const weak_ptr<Y>& rhs) noexcept { weak_ptr<T>(rhs).swap(*this); return *this; }
                      weak_ptr& operator=(weak_ptr&& rhs) noexcept         { weak_ptr<T>(std::move(rhs)).swap(*this); return *this; }
    template<class Y> weak_ptr& operator=(weak_ptr<Y>&& rhs) noexcept      { weak_ptr<T>(std::move(rhs)).swap(*this); return *this; }

    void swap(weak_ptr& rhs) noexcept {
        std::swap(m_ptr, rhs.m_ptr);
        std::swap(m_ctrl, rhs.m_ctrl);
    }

    ~weak_ptr() {
        decrement_weak_count();
    }

    void reset() noexcept {
        decrement_weak_count();
        m_ctrl = nullptr;
        m_ptr = nullptr;
    }

    long use_count() const noexcept {
        if (m_ctrl != nullptr) {
            return m_ctrl->m_use_count;
        } else {
            return 0;
        }
    }

    bool expired() const noexcept {
        return use_count() == 0;
    }

    shared_ptr<T> lock() const noexcept {
        int old_use_count = this->use_count();
        do {
            if (old_use_count == 0) {
                return nullptr;
            }
        } while (!m_ctrl->m_use_count.compare_exchange_weak(old_use_count, old_use_count+1));
        // We have successfully incremented the use-count!
        // Shove the bits into a shared_ptr and return.
        shared_ptr<T> result;
        result.m_ptr = this->m_ptr;
        result.m_ctrl = this->m_ctrl;
        return result;
    }

private:
    void increment_weak_count() noexcept {
        if (m_ctrl != nullptr) {
            ++m_ctrl->m_weak_count;
        }
    }
    void decrement_weak_count() noexcept {
        if (m_ctrl != nullptr) {
            int new_weak_count = --m_ctrl->m_weak_count;
            if (new_weak_count == 0) {
                delete m_ctrl;
            }
        }
    }

    detail::shared_ptr_control_block *m_ctrl = nullptr;
};

template<class T> weak_ptr(shared_ptr<T>) -> weak_ptr<T>;  // deduction guide

} // namespace scratch
