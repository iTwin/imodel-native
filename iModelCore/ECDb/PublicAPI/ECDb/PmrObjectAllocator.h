/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <array>
#include <memory_resource>
#include <type_traits>
#include <vector>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! A PMR-backed arena that placement-constructs objects deriving from @p Base and calls
//! their destructors in reverse order when Reset() is called.
//!
//! Features:
//!  - @p InlineBytes bytes of inline stack storage; objects are bump-allocated from it.
//!  - When the inline buffer is exhausted the underlying monotonic_buffer_resource
//!    spills to the heap via its upstream allocator (default: new/delete).
//!  - New<T>(...) requires T to derive from Base, constructs it in the arena, and
//!    registers T* (stored as Base*) for destruction.
//!  - Reset() calls std::destroy_at on each Base* in reverse construction order (virtual
//!    dispatch requires Base to have a virtual destructor), then releases all heap blocks
//!    back to the upstream and rewinds the bump pointer.
//!  - The destructor also calls Reset() so no objects are ever leaked.
//!  - Non-copyable, non-movable; hold by value.
//!
//! @note Unlike std::pmr::pool_resource, individual objects are never freed — only the
//!       whole arena can be reclaimed via Reset().  This makes the arena ideal for
//!       short-lived batches of polymorphic objects (e.g. per-row IECSqlValue trees).
//!
//! @par Thread safety
//!   Not thread-safe.  All calls from one thread only.
//!
//! @tparam Base        The common base class of all objects stored in this arena.
//!                     Must have a virtual destructor for correct polymorphic destruction.
//! @tparam InlineBytes Size of the inline stack buffer in bytes.  Default (64 KiB)
//!                     covers ~250-300 mid-sized objects without spilling to the heap.
//!
//! Example:
//! @code
//!   PmrObjectAllocator<IMyBase> arena;
//!   IMyBase* p = arena.New<MyConcrete>(arg1, arg2);
//!   // ... use p until the next Reset() ...
//!   arena.Reset();  // ~MyConcrete() called; memory reclaimed
//! @endcode
// @bsiclass
//+===============+===============+===============+===============+===============+======
template<typename Base, size_t InlineBytes = 64 * 1024>
struct PmrObjectAllocator final {
    static_assert(std::has_virtual_destructor<Base>::value,
                  "PmrObjectAllocator<Base>: Base must have a virtual destructor "
                  "so that std::destroy_at(Base*) dispatches to the derived destructor.");

    static constexpr size_t bytes = InlineBytes;

    std::array<std::byte, bytes>       m_storage {};
    std::pmr::monotonic_buffer_resource m_resource { m_storage.data(), bytes };
    std::vector<Base*>                  m_dtors;

    PmrObjectAllocator() = default;

    //! @internal For unit-testing only.
    //! Constructs the arena with a custom PMR upstream so tests can observe every
    //! heap allocation and deallocation via a TrackingUpstream or similar shim.
    //! Not for production use.
    explicit PmrObjectAllocator(std::pmr::memory_resource& upstream)
        : m_resource { m_storage.data(), bytes, &upstream } {}

    PmrObjectAllocator(PmrObjectAllocator const&)            = delete;
    PmrObjectAllocator(PmrObjectAllocator&&)                 = delete;
    PmrObjectAllocator& operator=(PmrObjectAllocator const&) = delete;
    PmrObjectAllocator& operator=(PmrObjectAllocator&&)      = delete;

    ~PmrObjectAllocator() { Reset(); }

    //! Placement-constructs a @p T in the arena's bump buffer and registers it for destruction.
    //! @p T must be derived from @p Base.
    //! @returns A non-owning pointer valid until the next Reset().
    //! @throws  Any exception thrown by T's constructor; the arena state remains valid.
    template<typename T, typename... Args>
    std::enable_if_t<std::is_base_of<Base, T>::value && std::is_constructible<T, Args...>::value, T*>
    New(Args&&... args) {
        std::pmr::polymorphic_allocator<std::byte> alloc(&m_resource);
        T* p = alloc.allocate_object<T>();
        try {
            ::new(p) T(std::forward<Args>(args)...);
        } catch (...) {
            alloc.deallocate_object<T>(p);
            throw;
        }
        try {
            m_dtors.push_back(p);
        } catch (...) {
            std::destroy_at(p);
            alloc.deallocate_object<T>(p);
            throw;
        }
        return p;
    }

    //! Destroys all registered objects in reverse construction order via virtual dispatch,
    //! releases all heap blocks back to the upstream, and rewinds the bump pointer.
    //! After Reset() the arena is ready for a new allocation cycle.
    void Reset() {
        for (auto it = m_dtors.rbegin(); it != m_dtors.rend(); ++it)
            std::destroy_at(*it);
        m_dtors.clear();
        auto* upstream = m_resource.upstream_resource();
        std::destroy_at(std::addressof(m_resource));
        ::new (std::addressof(m_resource))
            std::pmr::monotonic_buffer_resource(m_storage.data(), bytes, upstream);
    }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
