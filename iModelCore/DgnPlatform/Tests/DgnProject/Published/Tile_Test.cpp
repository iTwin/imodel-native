/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/Tile.h>
#include <DgnPlatform/TileIO.h>

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
TEST(ContentId, ToFromV1String)
    {
    ContentId id(NodeId::RootId(), 0, 1, ContentId::Flags::None);
    EXPECT_EQ(id.GetSizeMultiplier(), 1.0);
    EXPECT_EQ(0, id.GetDepth());

    Utf8String str = id.ToString();
    EXPECT_EQ(str, "0/0/0/0/1");

    ContentId rtId;
    EXPECT_TRUE(rtId.FromString(str.c_str(), 0));
    EXPECT_EQ(rtId, id);

    id = ContentId(1, 0, 0, 0, 5, 1, ContentId::Flags::None);
    EXPECT_EQ(id.GetSizeMultiplier(), 5.0);
    EXPECT_EQ(1, id.GetDepth());

    str = id.ToString();
    EXPECT_EQ(str, "1/0/0/0/5");
    EXPECT_TRUE(rtId.FromString(str.c_str(), 0));
    EXPECT_EQ(rtId, id);
    }

using TreeId = Tree::Id;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(IModelTileVersion, DetectIncompleteMajorBump)
    {
    // If this test fails, it probably means you changed IModelTile::Version::Current but did not update related code.
    auto curVersion = Tile::IO::IModelTile::Version::Current();
    EXPECT_EQ(Tile::IO::IModelTile::Version::FromMajorVersion(curVersion.m_major).m_major, curVersion.m_major);

    // Default constructor uses current major version.
    TreeId treeId;
    EXPECT_EQ(treeId.GetMajorVersion(), curVersion.m_major);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void expectEqualStrings(Utf8StringCR lhs, Utf8StringCR rhs)
    {
    EXPECT_TRUE(lhs == rhs) << lhs.c_str() << " vs " << rhs.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void expectEqualTreeIds(TreeId const& lhs, TreeId const& rhs)
    {
    EXPECT_EQ(lhs.GetModelId(), rhs.GetModelId());
    EXPECT_EQ(lhs.GetType(), rhs.GetType());
    EXPECT_EQ(lhs.GetMajorVersion(), rhs.GetMajorVersion());
    EXPECT_EQ(lhs.GetOmitEdges(), rhs.GetOmitEdges());
    EXPECT_EQ(lhs.GetFlags(), rhs.GetFlags());

    if (lhs.IsClassifier())
        EXPECT_TRUE(DoubleOps::WithinTolerance(lhs.GetClassifierExpansion(), rhs.GetClassifierExpansion(), 0.0000009));
    else if (lhs.IsAnimation())
        EXPECT_EQ(lhs.GetAnimationSourceId(), rhs.GetAnimationSourceId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void expectInvalidTreeId(Utf8CP str, DgnDbP db = nullptr)
    {
    TreeId id = TreeId::FromString(str, db);
    EXPECT_FALSE(id.IsValid()) << str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename... Args> static void expectInvalidTreeId(Args&&... args)
    {
    TreeId id(std::forward<Args>(args)...);
    EXPECT_FALSE(id.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void roundTripTreeId(Utf8CP inputIdStr, DgnDbP db = nullptr)
    {
    auto id = TreeId::FromString(inputIdStr, db);
    EXPECT_TRUE(id.IsValid());
    Utf8String outputIdStr = id.ToString();
    expectEqualStrings(outputIdStr, inputIdStr);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/19
//=======================================================================================
struct ExpectedId
{
    TreeId  m_id;
    Utf8CP  m_str;

    template<typename... Args> ExpectedId(Utf8CP str, Args&&... args) : m_id(std::forward<Args>(args)...), m_str(str) { }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(TreeId, RoundTrip)
    {
    constexpr auto kNone = Tree::Flags::None;
    constexpr auto kProjExt = Tree::Flags::UseProjectExtents;
    constexpr auto kLots = static_cast<Tree::Flags>(0xf6000a21);
    constexpr auto curMaj = Tile::IO::IModelTile::Version::Current().m_major;
    constexpr uint16_t v4 = 4;

    expectInvalidTreeId();
    expectInvalidTreeId(DgnModelId(), kNone, curMaj, false);
    expectInvalidTreeId(DgnModelId(), kNone, curMaj, 1.0, false);

    DgnModelId modelId(uint64_t(0x1c));
    expectInvalidTreeId(modelId, kNone, v4, 12.0, false); // must specify UseProjectExtents flag for volume classifier

    roundTripTreeId("0x123");
    roundTripTreeId("0xdef");
    roundTripTreeId("C:0.123456_0x1c");
    roundTripTreeId("CP:12345.678900_0x1c");
    roundTripTreeId("CP:0.000000_0x1c");
    roundTripTreeId("C:1.000001_0x1c");
    roundTripTreeId("C:333.333333_0x1c");
    roundTripTreeId("E:0_0x123");
    roundTripTreeId("E:0_0xabc02400def0");
    roundTripTreeId("A:0x123abd_0x420cf");
    roundTripTreeId("A:0x123abd_E:0_0x420cf");
    roundTripTreeId("0x10000000001");

    roundTripTreeId("4_0-0x1c");
    roundTripTreeId("4_1234-0x1c");
    roundTripTreeId("4_ab-0x1c");
    roundTripTreeId("4_ffff-0x1c");
    roundTripTreeId("4_ffffff-0x1c");
    roundTripTreeId("4_ffffffff-0x1c");

    expectInvalidTreeId("");
    expectInvalidTreeId("abc");
    expectInvalidTreeId("P:1.0_0x1c"); // unknown prefix
    expectInvalidTreeId("E:0_C:1.0_0x1c"); // classifier does not support "omit edges" specifier
    expectInvalidTreeId("C:1.0_E:0_0x1c");
    expectInvalidTreeId("E:0_"); // missing model Id
    expectInvalidTreeId("C:1.0_");
    expectInvalidTreeId("A:0x123_");
    expectInvalidTreeId("A:123_0x1c"); // incorrectly-formatted animation Id
    expectInvalidTreeId("C:-1.0_0x1c"); // negative classifier expansion
    expectInvalidTreeId("123"); // model Id not in hexadecimal format
    expectInvalidTreeId("E:0_123");
    expectInvalidTreeId("0"); // well-formed but invalid model Id
    expectInvalidTreeId("0x000123"); // leading zeroes
    expectInvalidTreeId("0x10000000000"); // invalid local Id
    expectInvalidTreeId("0x112233445566778899aabb"); // too many digits in model Id
    expectInvalidTreeId("E:0__0x1c"); // duplicate underscore
    expectInvalidTreeId("P:1.000000__0x1c");
    expectInvalidTreeId("E:1_0x1c");
    expectInvalidTreeId("E::0_0x1c");
    expectInvalidTreeId("A:0x123abd__E:0_0x1c");
    expectInvalidTreeId("A:0x123abd_E:0__0x1c");
    expectInvalidTreeId("0x1C"); // casing
    expectInvalidTreeId("0X1c");
    expectInvalidTreeId("e:0_0x1c");
    expectInvalidTreeId("0x1c "); // trailing whitespace
    expectInvalidTreeId("0x1c_"); // trailing underscore
    expectInvalidTreeId(" 0x1c"); // leading whitespace

    // major version 4+
    expectInvalidTreeId("4_0-");
    expectInvalidTreeId("4_0-abc");
    expectInvalidTreeId("4_0-P:1.0_0x1c"); // unknown prefix
    expectInvalidTreeId("4_0-E:0_C:1.0_0x1c"); // classifier does not support "omit edges" specifier
    expectInvalidTreeId("4_1-C:1.0_E:0_0x1c");
    expectInvalidTreeId("4_0-E:0_"); // missing model Id
    expectInvalidTreeId("4_0-C:1.0_");
    expectInvalidTreeId("4_0-A:0x123_");
    expectInvalidTreeId("4_0-A:123_0x1c"); // incorrectly-formatted animation Id
    expectInvalidTreeId("4_0-C:-1.0_0x1c"); // negative classifier expansion
    expectInvalidTreeId("");
    expectInvalidTreeId("3_0-0x1c"); // Id with explicit major version+flags only support for version 4+
    expectInvalidTreeId("10_0-0x1c"); // Unknown major version (at time of writing)
    expectInvalidTreeId("4-0x1c"); // missing flags
    expectInvalidTreeId("4_0xabc-0x1c"); // improperly-formatted flags
    expectInvalidTreeId("4_0-123"); // model Id not in hexadecimal format
    expectInvalidTreeId("4_0-E:0_123");
    expectInvalidTreeId("04_0-0x1c"); // leading zero in major version
    expectInvalidTreeId("12345_0-0x1c"); // too many digits in major version
    expectInvalidTreeId("4_01-0x1c"); // leading zero in flags
    expectInvalidTreeId("4_fffffffff-0x1c"); // too many digits in flags
    expectInvalidTreeId("C:12.340000_0-0x1c"); // must specify UseProjectExtents flag for volume classifier

    DgnElementId animId(uint64_t(0xf7));

    ExpectedId expectedIds[] =
        {
            { "0x1c", modelId, false },
            { "E:0_0x1c", modelId, true },
            { "A:0xf7_0x1c", modelId, false, animId },
            { "A:0xf7_E:0_0x1c", modelId, true, animId },
            { "C:12.345678_0x1c", modelId, 12.345678, false },
            { "CP:0.000300_0x1c", modelId, 0.0003, true },

            { "4_0-0x1c", modelId, kNone, v4, false },
            { "4_0-E:0_0x1c", modelId, kNone, v4, true },
            { "4_0-A:0xf7_0x1c", modelId, kNone, v4, false, animId },
            { "4_0-A:0xf7_E:0_0x1c", modelId, kNone, v4, true, animId },
            { "4_0-CP:12.345678_0x1c", modelId, kNone, v4, 12.345678, true },

            { "4_1-0x1c", modelId, kProjExt, v4, false },
            { "4_1-E:0_0x1c", modelId, kProjExt, v4, true },
            { "4_1-A:0xf7_0x1c", modelId, kProjExt, v4, false, animId },
            { "4_1-A:0xf7_E:0_0x1c", modelId, kProjExt, v4, true, animId },
            { "4_1-C:12.345678_0x1c", modelId, kProjExt, v4, 12.345678, false },
            { "4_1-CP:0.000300_0x1c", modelId, kProjExt, v4, 0.0003, true },

            { "4_f6000a21-0x1c", modelId, kLots, v4, false },
            { "4_f6000a21-E:0_0x1c", modelId, kLots, v4, true },
            { "4_f6000a21-A:0xf7_0x1c", modelId, kLots, v4, false, animId },
            { "4_f6000a21-A:0xf7_E:0_0x1c", modelId, kLots, v4, true, animId },
            { "4_f6000a21-C:12.345678_0x1c", modelId, kLots, v4, 12.345678, false },
            { "4_f6000a21-CP:0.000300_0x1c", modelId, kLots, v4, 0.0003, true },
        };

    for (auto const& expectedId : expectedIds)
        {
        auto const& id = expectedId.m_id;
        EXPECT_TRUE(id.IsValid());
        EXPECT_TRUE(id.GetModelId().IsValid());
        EXPECT_TRUE(Tile::IO::IModelTile::Version::IsKnownMajorVersion(id.GetMajorVersion()));
        EXPECT_EQ(id.GetAnimationSourceId().IsValid(), id.IsAnimation());

        auto actualStr = id.ToString();
        expectEqualStrings(actualStr, expectedId.m_str);

        auto roundTripId = TreeId::FromString(expectedId.m_str, nullptr);
        expectEqualTreeIds(roundTripId, id);

        switch (id.GetType())
            {
            case Tree::Type::VolumeClassifier:
                EXPECT_TRUE(id.GetUseProjectExtents());
                // fall-through...
            case Tree::Type::PlanarClassifier:
                EXPECT_TRUE(id.GetClassifierExpansion() > 0.0);
                EXPECT_TRUE(id.GetOmitEdges());
                break;
            case Tree::Type::Animation:
            case Tree::Type::Model:
                EXPECT_EQ(0.0, id.GetClassifierExpansion());
                break;
            default:
                EXPECT_TRUE(false) << "Unkown Tree::Type";
                break;
            }
        }

    // ###TODO Test validation against DgnDb:
    //  - Model does not exist / is not geometric
    //  - Animation Id not a valid display style
    //  - etc...
    }

