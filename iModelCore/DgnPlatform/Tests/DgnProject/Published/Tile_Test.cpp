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

