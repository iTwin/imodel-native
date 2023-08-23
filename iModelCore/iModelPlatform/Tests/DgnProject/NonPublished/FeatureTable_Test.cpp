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
TEST(FeatureTableTests, PackAndUnpack_SingleModel)
    {
      DgnModelId modelId(static_cast<uint64_t>(0x123));
#define MAKE_FEATURE(EID, SCID, CLS) Feature(modelId, DgnElementId(static_cast<uint64_t>(EID)), DgnSubCategoryId(static_cast<uint64_t>(SCID)), DgnGeometryClass:: CLS)

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

    FeatureTable table(FeatureTable::Type::SingleModel);
    for (auto const& feature : features)
        table.GetIndex(feature);

    EXPECT_EQ(_countof(features), table.GetNumIndices());
    PackedFeatureTable packed = table.Pack();
    ExpectEqualFeatureTables(table, packed.Unpack(modelId), true);

#undef MAKE_FEATURE
    }

// If the feature is not already present, its model Id must be >= any model Ids already inserted.
// We can query features with any model Id in any order as long as those features were already inserted in order by model Id.
TEST(FeatureTableTests, ThrowsIfInsertingModelsOutOfOrder) {
  FeatureTable ft(FeatureTable::Type::MultiModel);

  auto expectException = [&ft](uint64_t modelId, uint64_t elemId=1) {
    Feature feature(DgnModelId(modelId), DgnElementId(uint64_t(elemId)), DgnSubCategoryId(), DgnGeometryClass::Primary);
    auto size = ft.size();
    bool threw = false;
    try {
      ft.GetIndex(feature);
    } catch (std::runtime_error const& err) {
      EXPECT_EQ(err.what(), std::string("Features must be inserted in ascending order by model Id"));
      threw = true;
    } catch (...) {
      FAIL() << "Expected std::runtime_error";
    }

    EXPECT_TRUE(threw);
    EXPECT_EQ(ft.size(), size);
  };

  auto expectIndex = [&ft](uint64_t modelId, uint32_t expectedIndex) {
    Feature feature(DgnModelId(modelId), DgnElementId(uint64_t(1)), DgnSubCategoryId(), DgnGeometryClass::Primary);
    EXPECT_EQ(ft.GetIndex(feature), expectedIndex);
  };

  expectIndex(0x5, 0);
  expectIndex(0x5, 0);

  expectException(0x3);
  expectIndex(0x8, 1);
  expectException(0x7);
  expectException(0x5, 2);
  expectIndex(0xabc, 2);
}

Feature makeFeature(uint64_t model=0, uint64_t elem=0, uint64_t subcat=0, DgnGeometryClass cls=DgnGeometryClass::Primary) {
  return Feature(DgnModelId(model), DgnElementId(elem), DgnSubCategoryId(subcat), cls);
}

TEST(FeatureTableTests, ThrowsIfInsertingMultipleModelsIntoSingleModelTable) {
  FeatureTable ft(FeatureTable::Type::SingleModel);
  EXPECT_EQ(ft.GetIndex(makeFeature(1, 1)), 0);
  EXPECT_EQ(ft.GetIndex(makeFeature(1, 2)), 1);
  EXPECT_EQ(ft.GetIndex(makeFeature(1, 1)), 0);

  EXPECT_EQ(ft.size(), 2);
  bool threw = false;
  try {
    ft.GetIndex(makeFeature(2, 3));
  } catch (std::runtime_error const& err) {
    EXPECT_EQ(err.what(), std::string("Attempting to insert a second model into a single-model feature table"));
    threw = true;
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }

  EXPECT_TRUE(threw);
  EXPECT_EQ(ft.size(), 2);
}

TEST(FeatureTableTests, TracksLastFeatureInEachModel) {
  FeatureTable ft(FeatureTable::Type::MultiModel);

  auto expect = [&ft](std::vector<uint32_t>&& expected) {
    auto const& actual = ft.LastFeatureIndexPerModel();
    EXPECT_EQ(actual.size(), expected.size());
    for (size_t i = 0; i < actual.size(); i++)
      EXPECT_EQ(actual[i], expected[i]);
  };

  expect({ });

  ft.GetIndex(makeFeature(1, 11));
  expect({0});
  ft.GetIndex(makeFeature(1, 12));
  expect({1});
  ft.GetIndex(makeFeature(1, 10));
  expect({2});

  ft.GetIndex(makeFeature(2, 22));
  expect({2, 3});

  ft.GetIndex(makeFeature(3, 33));
  expect({2, 3, 4});
  ft.GetIndex(makeFeature(3, 32));
  expect({2, 3, 5});

  ft.GetIndex(makeFeature(7, 1));
  expect({2, 3, 5, 6});
  ft.GetIndex(makeFeature(7, 999));
  expect({2, 3, 5, 7});

  ft.clear();
  expect({ });
}
