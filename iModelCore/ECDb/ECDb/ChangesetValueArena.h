/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbTypes.h>
#include "ChangesetValue.h"
#include <array>
#include <memory_resource>
#include <vector>
#include <memory>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! POD handle used to thread the PMR arena allocator + destructor registry from
//! PreparedChangesetReader into ChangesetValueFactory without ownership.
//! Call New<T>(...) instead of std::make_unique<T>(...) everywhere in the factory.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetValueAllocator final {
    std::pmr::polymorphic_allocator<std::byte>  allocator;
    std::vector<ChangesetValue*>&               dtors;

    ChangesetValueAllocator(std::pmr::polymorphic_allocator<std::byte> alloc,
                           std::vector<ChangesetValue*>& dtorsRef)
        : allocator(alloc), dtors(dtorsRef) {}

    ChangesetValueAllocator(ChangesetValueAllocator const&)            = delete;
    ChangesetValueAllocator(ChangesetValueAllocator&&)                 = delete;
    ChangesetValueAllocator& operator=(ChangesetValueAllocator const&) = delete;
    ChangesetValueAllocator& operator=(ChangesetValueAllocator&&)      = delete;

    template<typename T, typename... Args>
    std::enable_if_t<std::is_base_of<ChangesetValue, T>::value && std::is_constructible<T, Args...>::value, T*>
    New(Args&&... args) {
        T* p = allocator.allocate_object<T>();
        try {
            ::new(p) T(std::forward<Args>(args)...);
        } catch (...) {
            allocator.deallocate_object<T>(p);
            throw;
        }
        try {
            dtors.push_back(p);
        } catch (...) {
            std::destroy_at(p);
            allocator.deallocate_object<T>(p);
            throw;
        }
        return p;
    }
};

//=======================================================================================
//! Owns the PMR inline buffer, monotonic resource, polymorphic allocator, and destructor
//! registry used to arena-allocate IECSqlValue objects during changeset reading.
//! PreparedChangesetReader holds one of these by composition.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetValueArena final {
    static constexpr size_t kBytes = 64 * 1024;  // covers ~250–300 properties/stage without overflow; raise if profiling shows frequent spill
    std::array<std::byte, kBytes>                 m_storage {};
    std::pmr::monotonic_buffer_resource           m_resource { m_storage.data(), kBytes };
    std::vector<ChangesetValue*>                  m_dtors;

    ChangesetValueArena() = default;

    //! @internal For unit-testing only.
    //! Constructs the arena with a custom PMR upstream so tests can observe every
    //! heap allocation and deallocation via a TrackingUpstream or similar shim.
    //! Not for production use.
    explicit ChangesetValueArena(std::pmr::memory_resource& upstream)
        : m_resource{ m_storage.data(), kBytes, &upstream } {}

    ChangesetValueArena(ChangesetValueArena const&)            = delete;
    ChangesetValueArena(ChangesetValueArena&&)                 = delete;
    ChangesetValueArena& operator=(ChangesetValueArena const&) = delete;
    ChangesetValueArena& operator=(ChangesetValueArena&&)      = delete;

    
    void Reset() {
        for (auto it = m_dtors.rbegin(); it != m_dtors.rend(); ++it)
            std::destroy_at(*it);
        m_dtors.clear();
        auto* upstream = m_resource.upstream_resource();
        std::destroy_at(std::addressof(m_resource));
        ::new (std::addressof(m_resource)) std::pmr::monotonic_buffer_resource(m_storage.data(), kBytes, upstream);
    }

    //! Returns an allocator handle for passing to ChangesetValueFactory.
    ChangesetValueAllocator MakeAllocator() { return { std::pmr::polymorphic_allocator<std::byte>(&m_resource), m_dtors }; }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
