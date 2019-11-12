//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>
#include <RealityPlatform/SpatioTemporalData.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include "../Common/RealityModFrameworkTestsCommon.h"

//=====================================================================================
//! @bsiclass                                  Remi.Charbonneau              06/2017
//=====================================================================================
class SpatioTemporalDataFixture: public ::testing::Test
    {
public:
    SpatioTemporalDataFixture() {};
    };


//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalDataFixture, EmptyJson)
    {
    auto dataset = SpatialEntityDataset::CreateFromJson("");
    EXPECT_TRUE(dataset->GetImageryGroup().empty());
    EXPECT_TRUE(dataset->GetImageryGroupR().empty());
    EXPECT_TRUE(dataset->GetTerrainGroup().empty());
    EXPECT_TRUE(dataset->GetTerrainGroupR().empty());
    }

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalDataFixture, InvalidJson)
{
    auto dataset = SpatialEntityDataset::CreateFromJson("INVALIDJSON");
    EXPECT_TRUE(dataset->GetImageryGroup().empty());
    EXPECT_TRUE(dataset->GetImageryGroupR().empty());
    EXPECT_TRUE(dataset->GetTerrainGroup().empty());
    EXPECT_TRUE(dataset->GetTerrainGroupR().empty());
}

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalDataFixture, InstancesNotRoot)
{
    auto dataset = SpatialEntityDataset::CreateFromJson(R"({"notInstances": "asroot"})");
    EXPECT_TRUE(dataset->GetImageryGroup().empty());
    EXPECT_TRUE(dataset->GetImageryGroupR().empty());
    EXPECT_TRUE(dataset->GetTerrainGroup().empty());
    EXPECT_TRUE(dataset->GetTerrainGroupR().empty());
}

TEST_F(SpatioTemporalDataFixture, LoadSampleJson)
    {
    auto json = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\SpatialEntities.json");
    auto dataset = SpatialEntityDataset::CreateFromJson(json.c_str());
    
    auto imageryGroup = dataset->GetImageryGroup();
    auto terrainGroup = dataset->GetTerrainGroup();

    EXPECT_EQ(imageryGroup.size(), 2);

    auto image = imageryGroup[0];
    EXPECT_EQ(image->GetIdentifier(), "14813");
    EXPECT_EQ(image->GetName(), "N39W093");
    EXPECT_EQ(image->GetResolution(), "24.2x30.922");

    DateTime date;
    DateTime::FromString(date, "2005-07-05T12:12:00");
    EXPECT_EQ(image->GetDate(), date);
    auto footPrint = image->GetFootprint();
    EXPECT_EQ(footPrint.size(), 5);
    EXPECT_NEAR(footPrint[0].longitude, -95.0, 0.0000001);
    EXPECT_NEAR(footPrint[0].latitude, 39.0, 0.0000001);
    EXPECT_NEAR(footPrint[3].longitude, -93.0, 0.0000001);
    EXPECT_NEAR(footPrint[3].latitude, 39.0, 0.0000001);

    image = imageryGroup[1];
    EXPECT_EQ(image->GetIdentifier(), "");
    EXPECT_EQ(image->GetName(), "");
    EXPECT_EQ(image->GetResolution(), "24.2x30.922");

    DateTime::FromString(date, "2005-07-05T12:12:00");
    EXPECT_EQ(image->GetDate(), date);
    footPrint = image->GetFootprint();
    EXPECT_EQ(footPrint.size(), 5);
    EXPECT_NEAR(footPrint[0].longitude, -97.0, 0.0000001);
    EXPECT_NEAR(footPrint[0].latitude, 39.0, 0.0000001);
    EXPECT_NEAR(footPrint[3].longitude, -93.0, 0.0000001);
    EXPECT_NEAR(footPrint[3].latitude, 39.0, 0.0000001);


    EXPECT_EQ(terrainGroup.size(), 2);

    auto terrain = terrainGroup[0];
    EXPECT_EQ(terrain->GetIdentifier(), "14812");
    EXPECT_EQ(terrain->GetName(), "N38W093");
    EXPECT_EQ(terrain->GetResolution(), "24.2x30.922");

    DateTime::FromString(date, "2005-07-05T00:00:00.000");
    EXPECT_EQ(terrain->GetDate(), date);
    footPrint = terrain->GetFootprint();
    EXPECT_EQ(footPrint.size(), 5);
    EXPECT_NEAR(footPrint[0].longitude, -92.0, 0.0000001);
    EXPECT_NEAR(footPrint[0].latitude, 39.0, 0.0000001);
    EXPECT_NEAR(footPrint[3].longitude, -93.0, 0.0000001);
    EXPECT_NEAR(footPrint[3].latitude, 39.0, 0.0000001);

    terrain = terrainGroup[1];
    EXPECT_EQ(terrain->GetIdentifier(), "");
    EXPECT_EQ(terrain->GetName(), "");
    EXPECT_EQ(terrain->GetResolution(), "24.2x30.922");

    DateTime::FromString(date, "2005-07-05T00:00:00.000");
    EXPECT_EQ(terrain->GetDate(), date);
    footPrint = terrain->GetFootprint();
    EXPECT_EQ(footPrint.size(), 5);
    EXPECT_NEAR(footPrint[0].longitude, -96.0, 0.0000001);
    EXPECT_NEAR(footPrint[0].latitude, 39.0, 0.0000001);
    EXPECT_NEAR(footPrint[3].longitude, -93.0, 0.0000001);
    EXPECT_NEAR(footPrint[3].latitude, 39.0, 0.0000001);
    }