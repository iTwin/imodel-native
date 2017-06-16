/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/RealityPlatform/SpatioTemporalSelectorTester.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/BeTest.h>
#include <RealityPlatform/SpatioTemporalSelector.h>
#include <RealityPlatform/SpatioTemporalData.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include "../Common/RealityModFrameworkTestsCommon.h"


USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsiclass                                  Remi.Charbonneau              06/2017
//=====================================================================================
class SpatioTemporalSelectorFixture : public ::testing::Test
{
public:
    SpatioTemporalSelectorFixture() {};
};

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalSelectorFixture, GetIdsFromJsonResolutionCriteria)
    {
    auto json = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\SpatialEntitiesBigList.json");

    auto regionOfInterest = new bvector<GeoPoint2d>();
    regionOfInterest->emplace_back(GeoPoint2d::From(-100, 40));
    regionOfInterest->emplace_back(GeoPoint2d::From(-100, 30));
    regionOfInterest->emplace_back(GeoPoint2d::From(-90, 30));
    regionOfInterest->emplace_back(GeoPoint2d::From(-90, 40));
    regionOfInterest->emplace_back(GeoPoint2d::From(-100, 40));

    // Low only return low, medium return medium+low and high return all.
    // they are based on the statistics of the whole set.

    auto Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Low, DateCriteria::UpToDate);
    EXPECT_EQ(Ids.size(), 3);
    bmap<int, string> test;
    test.size();

    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Medium, DateCriteria::UpToDate);
    EXPECT_EQ(Ids.size(), 5);

    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::High, DateCriteria::UpToDate);
    EXPECT_EQ(Ids.size(), 7);
    }

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalSelectorFixture, GetIdsFromJsonDateCriteria)
    {
    auto json = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\SpatialEntitiesBigList.json");

    auto regionOfInterest = new bvector<GeoPoint2d>();
    regionOfInterest->emplace_back(GeoPoint2d::From(-100, 40));
    regionOfInterest->emplace_back(GeoPoint2d::From(-100, 30));
    regionOfInterest->emplace_back(GeoPoint2d::From(-90, 30));
    regionOfInterest->emplace_back(GeoPoint2d::From(-90, 40));
    regionOfInterest->emplace_back(GeoPoint2d::From(-100, 40));

    DateTime date;

    auto Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Low, DateCriteria::Old);
    EXPECT_EQ(Ids[0], "2");
    EXPECT_EQ(Ids[1], "5");

    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Low, DateCriteria::Recent);
    EXPECT_EQ(Ids[0], "5");
    EXPECT_EQ(Ids[1], "2");

    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Medium, DateCriteria::UpToDate);
    EXPECT_EQ(Ids.size(), 4);

    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::High, DateCriteria::UpToDate);
    EXPECT_EQ(Ids.size(), 6);
    }