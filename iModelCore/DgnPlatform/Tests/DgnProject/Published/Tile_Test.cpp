/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/Tile.h>

USING_NAMESPACE_TILE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(NodeId, RootNode)
    {
    NodeId root;
    EXPECT_TRUE(root.IsRoot());
    EXPECT_EQ(root, NodeId::RootId());
    EXPECT_EQ(0, root.GetDepth());

    DRange3d rootRange = DRange3d::From(DPoint3d::From(0, 0, 0), DPoint3d::From(10, 5, 2));
    DRange3d computedRange = root.ComputeRange(rootRange, false);
    EXPECT_TRUE(rootRange.IsEqual(computedRange));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(NodeId, ChildNodes)
    {
    // Takes the lower half of every subdivision
    NodeId node(1, 0, 0, 0);
    EXPECT_EQ(node, node);

    NodeId parent = node.ComputeParentId();
    EXPECT_NE(node, parent);
    EXPECT_TRUE(parent.IsRoot());

    DRange3d rootRange = DRange3d::From(DPoint3d::From(0, 0, 0), DPoint3d::From(400, 420, 440));
    DRange3d nodeRange = node.ComputeRange(rootRange, false);
    EXPECT_FALSE(rootRange.IsEqual(nodeRange));
    EXPECT_TRUE(nodeRange.low.IsEqual(rootRange.low));
    EXPECT_TRUE(nodeRange.high.IsEqual(DPoint3d::From(200, 210, 220)));

    // Takes the upper half of each subdivision
    node = NodeId(1, 1, 1, 1);
    EXPECT_EQ(node, node);

    parent = node.ComputeParentId();
    EXPECT_NE(node, parent);
    EXPECT_TRUE(parent.IsRoot());

    nodeRange = node.ComputeRange(rootRange, false);
    EXPECT_FALSE(rootRange.IsEqual(nodeRange));
    EXPECT_TRUE(nodeRange.high.IsEqual(rootRange.high));
    EXPECT_TRUE(nodeRange.low.IsEqual(DPoint3d::From(200, 210, 220)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(NodeId, GrandchildNodes)
    {
    DRange3d rootRange;
    rootRange.low = DPoint3d::From(0, 0, 0);
    rootRange.high = DPoint3d::From(400, 420, 440);

    // Parent is (1, 0, 0, 0) - takes lower half of every subdivision
    // Depth 2, takes lower half of every subdivision
    NodeId lllChild(2, 0, 0, 0);

    // Confirm parent ID
    NodeId parent = lllChild.ComputeParentId();
    EXPECT_EQ(parent, NodeId(1, 0, 0, 0));

    // Confirm parent range
    DRange3d parentRange = parent.ComputeRange(rootRange, false);
    EXPECT_TRUE(parentRange.low.IsEqual(rootRange.low));
    EXPECT_TRUE(parentRange.high.IsEqual(DPoint3d::From(200, 210, 220)));

    // Confirm child range
    DRange3d childRange = lllChild.ComputeRange(rootRange, false);
    EXPECT_TRUE(childRange.low.IsEqual(parentRange.low));
    EXPECT_TRUE(childRange.high.IsEqual(DPoint3d::From(100, 105, 110)));

    // Depth 2, takes upper half of every subdivision
    NodeId uuuChild(2, 1, 1, 1);
    parent = uuuChild.ComputeParentId();
    EXPECT_EQ(parent, NodeId(1, 0, 0, 0));

    // Confirm child range
    childRange = uuuChild.ComputeRange(rootRange, false);
    EXPECT_TRUE(childRange.low.IsEqual(DPoint3d::From(100, 105, 110)));
    EXPECT_TRUE(childRange.high.IsEqual(parentRange.high));

    // Depth 2, takes upper, lower, upper
    NodeId uluChild(2, 1, 0, 1);
    parent = uluChild.ComputeParentId();
    EXPECT_EQ(parent, NodeId(1, 0, 0, 0));

    // Confirm child range
    childRange = uluChild.ComputeRange(rootRange, false);
    EXPECT_TRUE(childRange.low.IsEqual(DPoint3d::From(100, 0, 110)));
    EXPECT_TRUE(childRange.high.IsEqual(DPoint3d::From(200, 105, 220)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(NodeId, UpperGrandChildNodes)
    {
    DRange3d rootRange = DRange3d::From(DPoint3d::From(0, 0, 0), DPoint3d::From(400, 420, 440));

    // Parent is (1, 1, 1, 1) - takes upper half of every subdivision
    // Child takes upper half of every subdivision
    NodeId uuuChild(2, 3, 3, 3);
    NodeId parent = uuuChild.ComputeParentId();
    EXPECT_EQ(parent, NodeId(1, 1, 1, 1));

    DRange3d parentRange = parent.ComputeRange(rootRange, false);
    EXPECT_TRUE(parentRange.low.IsEqual(DPoint3d::From(200, 210, 220)));
    EXPECT_TRUE(parentRange.high.IsEqual(rootRange.high));

    DRange3d childRange = uuuChild.ComputeRange(rootRange, false);
    EXPECT_TRUE(childRange.low.IsEqual(DPoint3d::From(300, 315, 330)));
    EXPECT_TRUE(childRange.high.IsEqual(parentRange.high));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void ExpectSubdivision(DRange3dCR parentRange, DRange3dCR childRange, bool i, bool j, bool k)
    {
    double hx = parentRange.low.x + (parentRange.high.x - parentRange.low.x) * 0.5,
           hy = parentRange.low.y + (parentRange.high.y - parentRange.low.y) * 0.5,
           hz = parentRange.low.z + (parentRange.high.z - parentRange.low.z) * 0.5;
    DPoint3d lo = DPoint3d::From(i ? hx : parentRange.low.x, j ? hy : parentRange.low.y, k ? hz : parentRange.low.z);
    DPoint3d hi = DPoint3d::From(i ? parentRange.high.x : hx, j ? parentRange.high.y : hy, k ? parentRange.high.z : hz);
    EXPECT_TRUE(childRange.low.IsEqual(lo));
    EXPECT_TRUE(childRange.high.IsEqual(hi));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
struct EqualRange
{
    bool operator()(DRange3dCR lhs, DRange3dCR rhs) const
        {
        if (lhs.low.x != rhs.low.x) return lhs.low.x < rhs.low.x;
        if (lhs.low.y != rhs.low.y) return lhs.low.y < rhs.low.y;
        if (lhs.low.z != rhs.low.z) return lhs.low.z < rhs.low.z;
        if (lhs.high.x != rhs.high.x) return lhs.high.x < rhs.high.x;
        if (lhs.high.y != rhs.high.y) return lhs.high.y < rhs.high.y;
        if (lhs.high.z != rhs.high.z) return lhs.high.z < rhs.high.z;
        return false;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void testSubdivision(DRange3dCR rootRange, DRange3d parentRange, NodeId parentId, bset<DRange3d, EqualRange>& ranges, uint32_t maxDepth)
    {
    for (uint32_t i = 0; i < 2; i++)
        {
        for (uint32_t j = 0; j < 2; j++)
            {
            for (uint32_t k = 0; k < 2; k++)
                {
                bool bI = i != 0, bJ = j != 0, bK = k != 0;
                NodeId childId = parentId.ComputeChildId(bI, bJ, bK);
                DRange3d childRange = childId.ComputeRange(rootRange, false);
                ExpectSubdivision(parentRange, childRange, bI, bJ, bK);
                auto inserted = ranges.insert(childRange);
                EXPECT_TRUE(inserted.second);

                NodeId parentId2 = childId.ComputeParentId();
                EXPECT_EQ(parentId, parentId2);
                DRange3d parentRange2 = parentId.ComputeRange(rootRange, false);
                EXPECT_TRUE(parentRange.IsEqual(parentRange2));

                if (childId.GetDepth() < maxDepth)
                    testSubdivision(rootRange, childRange, childId, ranges, maxDepth);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(NodeId, Subdivision)
    {
    DRange3d rootRange = DRange3d::From(DPoint3d::From(0, 0, 0), DPoint3d::From(480, 440, 400));
    bset<DRange3d, EqualRange> ranges;
    ranges.insert(rootRange);
    NodeId parentId = NodeId::RootId();

    static constexpr uint32_t maxDepth = 6;
    testSubdivision(rootRange, rootRange, parentId, ranges, maxDepth);

    uint64_t nTiles = 0;
    for (uint32_t i = 0; i <= maxDepth; i++)
        nTiles += static_cast<uint32_t>(pow(8, i));

    EXPECT_EQ(ranges.size(), nTiles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ContentId, ToFromString)
    {
    ContentId id(NodeId::RootId(), 0);
    EXPECT_EQ(id.GetSizeMultiplier(), 1.0);
    EXPECT_EQ(0, id.GetDepth());

    Utf8String str = id.ToString();
    EXPECT_EQ(str, "0/0/0/0/1");

    ContentId rtId;
    EXPECT_TRUE(rtId.FromString(str.c_str()));
    EXPECT_EQ(rtId, id);

    id = ContentId(1, 0, 0, 0, 5);
    EXPECT_EQ(id.GetSizeMultiplier(), 5.0);
    EXPECT_EQ(1, id.GetDepth());

    str = id.ToString();
    EXPECT_EQ(str, "1/0/0/0/5");
    EXPECT_TRUE(rtId.FromString(str.c_str()));
    EXPECT_EQ(rtId, id);
    }

