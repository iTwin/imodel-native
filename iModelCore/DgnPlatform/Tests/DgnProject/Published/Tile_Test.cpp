/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/Tile_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

    // Depth 2, takes lower half of every subdivision
    NodeId lllChild(2, 0, 0, 0);

    // Confirm parent ID
    NodeId parent = lllChild.ComputeParentId();
    EXPECT_EQ(parent, NodeId(1, 0, 0, 0));

    // COnfirm parent range
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

