#pragma once

#include "scratch/bits/memory-resource/memory-resource.h"
#include "scratch/bits/containers/fancy-polymorphic-allocator.h"
#include "scratch/bits/memory-resource/set-default-resource.h"

namespace scratch::pmr {

template<class T>
class polymorphic_allocator : public fancy_polymorphic_allocator<T, memory_resource, void*> {
public:
    using fancy_polymorphic_allocator<T, memory_resource, void*>::fancy_polymorphic_allocator;

    polymorphic_allocator() : polymorphic_allocator(get_default_resource()) {}

    polymorphic_allocator select_on_container_copy_construction() const {
        return polymorphic_allocator();
    }
};

} // namespace scratch::pmr
