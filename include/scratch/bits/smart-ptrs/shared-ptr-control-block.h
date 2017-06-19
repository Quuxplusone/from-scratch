#pragma once

#include <atomic>
#include <utility>

namespace scratch::detail {

struct shared_ptr_control_block {
    virtual void delete_controlled_object() noexcept = 0;
    virtual ~shared_ptr_control_block() = default;

    std::atomic<int> m_use_count {1};
};

template<class DeletionLambda>
struct shared_ptr_control_block_impl : public shared_ptr_control_block {
    shared_ptr_control_block_impl(DeletionLambda&& d) : m_deleter(std::move(d)) {}
    void delete_controlled_object() noexcept override { m_deleter(); }

    DeletionLambda m_deleter;
};

} // namespace scratch::detail
