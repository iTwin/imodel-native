/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/Render.h>

USING_NAMESPACE_BENTLEY_RENDER

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExpectEqualFeatureTables(FeatureTableCR base, FeatureTableCR comp, bool expectEqualIndices = false)
    {
    EXPECT_EQ(base.size(), comp.size());
    EXPECT_EQ(base.GetMaxFeatures(), comp.GetMaxFeatures());
    EXPECT_EQ(base.IsUniform(), comp.IsUniform());

    for (auto kvp : base)
        {
        uint32_t compIndex;
        EXPECT_TRUE(comp.FindIndex(compIndex, kvp.first));
        if (expectEqualIndices)
            EXPECT_EQ(kvp.second, compIndex);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FeatureTableTests, PackAndUnpack)
    {
#define MAKE_FEATURE(EID, SCID, CLS) Feature(DgnElementId(static_cast<uint64_t>(EID)), DgnSubCategoryId(static_cast<uint64_t>(SCID)), DgnGeometryClass:: CLS)

    uint64_t largeIdBase = 0xABCDABCDABCDABCD;
    Feature features[] =
        {
        MAKE_FEATURE(1, 1, Primary),
        MAKE_FEATURE(2, 1, Primary),
        MAKE_FEATURE(3, 1, Construction),
        MAKE_FEATURE(4, largeIdBase, Primary),
        MAKE_FEATURE(largeIdBase+1, 99, Construction),
        MAKE_FEATURE(largeIdBase-1, 200, Primary),
        MAKE_FEATURE(largeIdBase-5, largeIdBase+5, Construction),
        MAKE_FEATURE(2, largeIdBase, Primary),
        MAKE_FEATURE(1, 1, Construction),
        };

    FeatureTable table(DgnModelId(static_cast<uint64_t>(1234)), 100);
    for (auto const& feature : features)
        table.GetIndex(feature);

    EXPECT_EQ(_countof(features), table.GetNumIndices());
    PackedFeatureTable packed = table.Pack();
    ExpectEqualFeatureTables(table, packed.Unpack(), true);

#undef MAKE_FEATURE
    }
