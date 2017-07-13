#pragma once

#include "scratch/bits/memory-resource/memory-resource.h"
#include "scratch/bits/memory-resource/new-delete-resource.h"

#include <atomic>

namespace scratch::pmr::detail {

inline auto& atomic_default_resource() noexcept {
    static std::atomic<memory_resource *> instance(new_delete_resource());
    return instance;
}

} // namespace scratch::pmr::detail

namespace scratch::pmr {

memory_resource *get_default_resource() noexcept {
    return detail::atomic_default_resource().load();
}

memory_resource *set_default_resource(memory_resource *mr) noexcept {
    return detail::atomic_default_resource().exchange(mr);
}

} // namespace scratch::pmr
