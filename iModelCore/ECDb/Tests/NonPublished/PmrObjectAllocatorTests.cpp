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
        EXPECT_EQ(0u, arena.GetDtorCount()) << "Registry must be empty at start of cycle " << cycle;
        int beforeCycle = totalCalls;
        {
        arena.New<Counter>(totalCalls);
        arena.New<Counter>(totalCalls);
        }
        EXPECT_EQ(2u, arena.GetDtorCount()) << "Exactly 2 entries per cycle";
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

    EXPECT_EQ(0u, arena.GetDtorCount()) << "Registry must be empty before any New() calls";
    arena.New<Noop>();
    EXPECT_EQ(1u, arena.GetDtorCount());
    arena.New<Noop>();
    EXPECT_EQ(2u, arena.GetDtorCount());
    arena.New<Noop>();
    EXPECT_EQ(3u, arena.GetDtorCount());

    arena.Reset();
    EXPECT_EQ(0u, arena.GetDtorCount());
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

    auto const* storageBegin = reinterpret_cast<uint8_t const*>(arena.GetStorageData());
    auto const* storageEnd   = storageBegin + arena.GetStorageSize();
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
    struct Oversized : TestChangesetValue { std::array<std::byte, 25> data {}; };
    PmrObjectAllocator<Oversized, 20> arena;
    Oversized* p = arena.New<Oversized>();

    auto const* storageBegin = reinterpret_cast<uint8_t const*>(arena.GetStorageData());
    auto const* storageEnd   = storageBegin + arena.GetStorageSize();
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
    EXPECT_EQ(1u, arena.GetDtorCount());
    arena.New<Noop>();
    EXPECT_EQ(2u, arena.GetDtorCount()) << "Second New() must append to the same registry";

    arena.Reset();
    EXPECT_EQ(0u, arena.GetDtorCount());
    }

//---------------------------------------------------------------------------------------
// Allocation order and tree shape:
//
//   leaf1(1)  leaf2(2)  ──  midA(5) ──┐
//   leaf3(3)  leaf4(4)  ──  midB(6) ──┼──  root(8)
//   leaf5(7)            ──────────────┘
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetArenaTests, Arena_DeepHierarchy_DestructionOrderIsStrictlyReversed)
    {
    //! Shared destruction log.  Each entry is the "id" of the object destroyed.
    std::vector<int> destructionLog;

    // Base node — records its own id on destruction via the most-derived dtor only
    // (recordedByDerived flag prevents the base dtor from double-logging).
    struct NodeB : TestChangesetValue
        {
        int              id;
        std::vector<int>& log;
        bool             recordedByDerived { false };
        explicit NodeB(int id_, std::vector<int>& log_) : id(id_), log(log_) {}
        ~NodeB() override { if (!recordedByDerived) log.push_back(id); }
        };

    // Leaf — terminal node, no children.
    struct LeafB final : NodeB
        {
        int payload;
        LeafB(int id_, std::vector<int>& log_, int p) : NodeB(id_, log_), payload(p) {}
        ~LeafB() { recordedByDerived = true; log.push_back(id); }
        };

    // Mid-level node — holds references to two leaf children and verifies both are
    // still alive during its own destruction.
    struct MidB final : NodeB
        {
        NodeB const& left;
        NodeB const& right;
        MidB(int id_, std::vector<int>& log_, NodeB const& l, NodeB const& r)
            : NodeB(id_, log_), left(l), right(r) {}
        ~MidB()
            {
            EXPECT_EQ(left.id,  left.id)  << "MidB left child accessed after its own destruction";
            EXPECT_EQ(right.id, right.id) << "MidB right child accessed after its own destruction";
            recordedByDerived = true;
            log.push_back(id);
            }
        };

    // Root node — holds references to two mid-level nodes and one cross-level leaf
    // (leaf5, allocated after the mid nodes) and verifies all three are still alive
    // during its own destruction.
    struct RootB final : NodeB
        {
        NodeB const& midLeft;
        NodeB const& midRight;
        NodeB const& extraLeaf;
        RootB(int id_, std::vector<int>& log_, NodeB const& ml, NodeB const& mr, NodeB const& ex)
            : NodeB(id_, log_), midLeft(ml), midRight(mr), extraLeaf(ex) {}
        ~RootB()
            {
            EXPECT_EQ(midLeft.id,   midLeft.id)   << "RootB midLeft accessed after its destruction";
            EXPECT_EQ(midRight.id,  midRight.id)  << "RootB midRight accessed after its destruction";
            EXPECT_EQ(extraLeaf.id, extraLeaf.id) << "RootB extraLeaf accessed after its destruction";
            recordedByDerived = true;
            log.push_back(id);
            }
        };

    PmrObjectAllocator<NodeB> arena;

    // Level 0 — four leaves feeding into the two mid nodes.
    LeafB* leaf1 = arena.New<LeafB>(1, destructionLog, 10);
    LeafB* leaf2 = arena.New<LeafB>(2, destructionLog, 20);
    LeafB* leaf3 = arena.New<LeafB>(3, destructionLog, 30);
    LeafB* leaf4 = arena.New<LeafB>(4, destructionLog, 40);

    // Level 1 — two mid nodes each grouping a pair of leaves.
    MidB* midA = arena.New<MidB>(5, destructionLog, *leaf1, *leaf2);
    MidB* midB = arena.New<MidB>(6, destructionLog, *leaf3, *leaf4);

    // Cross-level leaf — allocated after the mid nodes but attached directly to root,
    // so its id (7) sits between midB(6) and root(8) in the destruction sequence.
    LeafB* leaf5 = arena.New<LeafB>(7, destructionLog, 50);

    // Level 2 — root referencing both mid nodes and the cross-level leaf.
    RootB* root = arena.New<RootB>(8, destructionLog, *midA, *midB, *leaf5);

    ASSERT_NE(nullptr, leaf1); ASSERT_NE(nullptr, leaf2);
    ASSERT_NE(nullptr, leaf3); ASSERT_NE(nullptr, leaf4);
    ASSERT_NE(nullptr, midA);  ASSERT_NE(nullptr, midB);
    ASSERT_NE(nullptr, leaf5); ASSERT_NE(nullptr, root);
    EXPECT_EQ(8u, arena.GetDtorCount());

    arena.Reset();

    // Strictly reverse-construction order: 8, 7, 6, 5, 4, 3, 2, 1.
    ASSERT_EQ(8u, destructionLog.size()) << "Exactly 8 destructors must have fired";
    EXPECT_EQ(8, destructionLog[0]) << "root  must be destroyed first  (last constructed)";
    EXPECT_EQ(7, destructionLog[1]) << "leaf5 must be destroyed second";
    EXPECT_EQ(6, destructionLog[2]) << "midB  must be destroyed third";
    EXPECT_EQ(5, destructionLog[3]) << "midA  must be destroyed fourth";
    EXPECT_EQ(4, destructionLog[4]) << "leaf4 must be destroyed fifth";
    EXPECT_EQ(3, destructionLog[5]) << "leaf3 must be destroyed sixth";
    EXPECT_EQ(2, destructionLog[6]) << "leaf2 must be destroyed seventh";
    EXPECT_EQ(1, destructionLog[7]) << "leaf1 must be destroyed last   (first constructed)";
    }

END_ECDBUNITTESTS_NAMESPACE
