/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECDb/PmrObjectAllocator.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

// ============================================================================
// Custom PMR upstream allocator that counts every allocate/deallocate call.
// Used to prove that monotonic_buffer_resource::release() frees all heap
// memory it acquired from its upstream.  Thread-unsafe — single-threaded tests only.
// @bsiclass
// ============================================================================
struct TrackingUpstream final : std::pmr::memory_resource
    {
    size_t allocatedBytes   = 0;
    size_t deallocatedBytes = 0;
    int    allocCount       = 0;
    int    deallocCount     = 0;

    bool AllBalanced() const
        {
        return allocCount == deallocCount && allocatedBytes == deallocatedBytes;
        }

private:
    void* do_allocate(size_t bytes, size_t align) override
        {
        allocatedBytes += bytes;
        ++allocCount;
        return std::pmr::new_delete_resource()->allocate(bytes, align);
        }

    void do_deallocate(void* p, size_t bytes, size_t align) override
        {
        deallocatedBytes += bytes;
        ++deallocCount;
        std::pmr::new_delete_resource()->deallocate(p, bytes, align);
        }

    bool do_is_equal(std::pmr::memory_resource const& other) const noexcept override
        {
        return this == &other;
        }
    };

// ============================================================================
// Minimal concrete ChangesetValue subclass used as a base for test types.
// All arena allocations must be ChangesetValue subclasses; this gives each
// test struct the required base without importing any production column info.
// @bsiclass
// ============================================================================
struct TestChangesetValue
    {
   virtual ~TestChangesetValue() = default;
    };

// ============================================================================
// PmrObjectAllocator<Base> — raw memory tests
// These tests exercise the allocator machinery directly without involving the
// full ChangesetReader pipeline.
// @bsiclass
// ============================================================================
struct ChangesetArenaTests : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// Allocate an object whose destructor writes a known tombstone value to its own memory.
// Capture the raw address before Reset(), then verify the tombstone is present after
// Reset() — proving the destructor was actually called (not skipped).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetArenaTests, Arena_DestructorWritesTombstoneAtCapturedAddress)
    {
    //! Object whose destructor stamps a sentinel so we can confirm it was called.
    struct Sentinel: TestChangesetValue
        {
        volatile int32_t data = 0x12345678;
        ~Sentinel() 
        { 
            data = static_cast<int32_t>(0xDEADBEEF); 
        }
        };

    PmrObjectAllocator<Sentinel> arena;
    Sentinel* p = arena.New<Sentinel>();
    volatile int32_t* dataAddr = &p->data;

    EXPECT_EQ(0x12345678, p->data) << "Object must hold initial value before Reset()";
    EXPECT_EQ(1u, arena.m_dtors.size());

    arena.Reset();

    // Accessing memory through the raw address after destruction is technically UB,
    // but with volatile and no subsequent allocation this reliably verifies the dtor.
    int32_t const tombstone = *dataAddr;
    EXPECT_EQ(static_cast<int32_t>(0xDEADBEEF), tombstone)
        << "Destructor must have written the tombstone — dtor was not called if this fails";
    EXPECT_EQ(0u, arena.m_dtors.size()) << "Dtor registry must be empty after Reset()";
    }

//---------------------------------------------------------------------------------------
// After Reset(), monotonic_buffer_resource::release() rewinds the bump pointer to the
// start of the inline buffer.  The first allocation in the next cycle therefore lands
// at the same address as the first allocation in the previous cycle.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetArenaTests, Arena_BumpPointerRewound_SameAddressAfterReset)
    {
    struct Tag : TestChangesetValue { int id = 0; };

    PmrObjectAllocator<Tag> arena;
    void* addrCycle1;
    {
    addrCycle1 = arena.New<Tag>();
    }
    arena.Reset();

    void* addrCycle2;
    {
    addrCycle2 = arena.New<Tag>();
    }
    arena.Reset();

    EXPECT_EQ(addrCycle1, addrCycle2)
        << "release() must rewind the bump pointer so cycle 2 reuses cycle 1's address";
    }

//---------------------------------------------------------------------------------------
// Construct a PmrObjectAllocator with a TrackingUpstream so every heap allocation and
// deallocation is observed directly.  Allocate three 22 KiB blocks: the first two fit
// in the 64 KiB inline buffer, the third spills to the heap.  After Reset() the
// upstream must report that every byte it handed out has been returned — no heap leak.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetArenaTests, Arena_HeapAllocationsFreedAfterRelease_NoLeak)
    {
    TrackingUpstream upstream;
    struct Block: TestChangesetValue { std::array<std::byte, 22 * 1024> data {}; };
    // Construct the arena with a custom upstream so we can observe its heap traffic.
    // This uses the @internal constructor added specifically for testing.
    PmrObjectAllocator<Block> arena { upstream };

    // Each block is 22 KiB.  Two blocks consume 44 KiB of the 64 KiB inline buffer;
    // the third (would need 22 KiB but only 20 KiB remain) forces the monotonic
    // resource to request a new heap block from the upstream.
    arena.New<Block>();
    arena.New<Block>();
    arena.New<Block>();

    EXPECT_GT(upstream.allocCount, 0)   << "Inline buffer must be exhausted so heap is used";
    EXPECT_EQ(0, upstream.deallocCount) << "Nothing freed yet — Reset() not called";

    // Reset() calls all registered dtors then invokes m_resource.release(), which
    // returns every upstream-allocated buffer back to the TrackingUpstream.
    arena.Reset();

    EXPECT_TRUE(upstream.AllBalanced())
        << "Every byte allocated from the upstream must be returned after Reset(): "
        << "allocated=" << upstream.allocatedBytes
        << " freed=" << upstream.deallocatedBytes;
    }

//---------------------------------------------------------------------------------------
// Verify the destructor is called exactly once per allocated object — not zero times
// (missed) and not twice (double-free).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetArenaTests, Arena_DtorCalledExactlyOnce_NotDoubleNotMissed)
    {
    int callCount = 0;
    struct Counter: TestChangesetValue
        {
        int& count;
        explicit Counter(int& c) : TestChangesetValue(), count(c) {}
        ~Counter() { ++count; }
        };

    PmrObjectAllocator<Counter> arena;
    {
    arena.New<Counter>(callCount);
    arena.New<Counter>(callCount);
    arena.New<Counter>(callCount);
    }
    EXPECT_EQ(0, callCount) << "No dtor must fire before Reset()";

    arena.Reset();
    EXPECT_EQ(3, callCount) << "Each object's dtor must fire exactly once";

    // A second Reset() on a cleared arena must not trigger any additional calls.
    arena.Reset();
    EXPECT_EQ(3, callCount) << "Second Reset() on empty arena must not re-fire any dtor";
    }

//---------------------------------------------------------------------------------------
// Run N allocate→Reset cycles.  The dtor registry must be empty at the start of each
// new cycle — proving there is no accumulation of stale registrations across resets.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetArenaTests, Arena_MultipleResetCycles_NoDtorAccumulation)
    {
    int totalCalls = 0;
    struct Counter: TestChangesetValue { int& count; explicit Counter(int& c) : TestChangesetValue(), count(c) {} ~Counter() { ++count; } };

    PmrObjectAllocator<Counter> arena;
    for (int cycle = 0; cycle < 5; ++cycle)
        {
        EXPECT_EQ(0u, arena.m_dtors.size()) << "Registry must be empty at start of cycle " << cycle;
        int beforeCycle = totalCalls;
        {
        arena.New<Counter>(totalCalls);
        arena.New<Counter>(totalCalls);
        }
        EXPECT_EQ(2u, arena.m_dtors.size()) << "Exactly 2 entries per cycle";
        arena.Reset();
        EXPECT_EQ(beforeCycle + 2, totalCalls) << "2 dtors must have fired in cycle " << cycle;
        }
    EXPECT_EQ(10, totalCalls) << "5 cycles × 2 dtors = 10 total destructor calls";
    }

// ============================================================================
// PmrObjectAllocator — direct New() tests
// ============================================================================

//---------------------------------------------------------------------------------------
// Every call to New<T>() must append exactly one entry to m_dtors.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetArenaTests, Allocator_EachNewCallRegistersExactlyOneDtor)
    {
    struct Noop: TestChangesetValue { ~Noop() {} };

    PmrObjectAllocator<Noop> arena;

    EXPECT_EQ(0u, arena.m_dtors.size());
    arena.New<Noop>();
    EXPECT_EQ(1u, arena.m_dtors.size());
    arena.New<Noop>();
    EXPECT_EQ(2u, arena.m_dtors.size());
    arena.New<Noop>();
    EXPECT_EQ(3u, arena.m_dtors.size());

    arena.Reset();
    EXPECT_EQ(0u, arena.m_dtors.size());
    }

//---------------------------------------------------------------------------------------
// A small allocation must come from the 64 KiB inline buffer (address inside m_storage).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetArenaTests, Allocator_SmallAlloc_AddressWithinInlineStorage)
    {
    struct Small: TestChangesetValue { int x = 0; };
    PmrObjectAllocator<Small> arena;

    Small* p = arena.New<Small>();

    auto const* storageBegin = reinterpret_cast<uint8_t const*>(arena.m_storage.data());
    auto const* storageEnd   = storageBegin + arena.m_storage.size();
    auto const* pBytes       = reinterpret_cast<uint8_t const*>(p);

    EXPECT_GE(pBytes, storageBegin) << "Small allocation must reside inside the inline buffer";
    EXPECT_LT(pBytes, storageEnd)   << "Small allocation must reside inside the inline buffer";

    arena.Reset();
    }

//---------------------------------------------------------------------------------------
// An allocation that exceeds the entire inline buffer must be served from the heap
// (address outside m_storage).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetArenaTests, Allocator_OversizedAlloc_SpillsToHeap_AddressOutsideStorage)
    {
    // Request more bytes than the entire inline buffer in one allocation.
    struct Oversized : TestChangesetValue { std::array<std::byte, 21> data {}; };
    PmrObjectAllocator<Oversized, 20> arena;
    Oversized* p = arena.New<Oversized>();

    auto const* storageBegin = reinterpret_cast<uint8_t const*>(arena.m_storage.data());
    auto const* storageEnd   = storageBegin + arena.m_storage.size();
    auto const* pBytes       = reinterpret_cast<uint8_t const*>(p);
    bool const inInline      = (pBytes >= storageBegin && pBytes < storageEnd);

    EXPECT_FALSE(inInline)
        << "Allocation exceeding the inline buffer must be served from the heap";

    arena.Reset(); // must free the heap block without crashing
    }

//---------------------------------------------------------------------------------------
// Multiple New<T>() calls on the same arena all accumulate in a single m_dtors registry.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetArenaTests, Arena_MultipleNewCalls_ShareSingleDtorRegistry)
    {
    struct Noop: TestChangesetValue { ~Noop() {} };

    PmrObjectAllocator<Noop> arena;
    arena.New<Noop>();
    EXPECT_EQ(1u, arena.m_dtors.size());
    arena.New<Noop>();
    EXPECT_EQ(2u, arena.m_dtors.size()) << "Second New() must append to the same registry";

    arena.Reset();
    EXPECT_EQ(0u, arena.m_dtors.size());
    }

END_ECDBUNITTESTS_NAMESPACE
