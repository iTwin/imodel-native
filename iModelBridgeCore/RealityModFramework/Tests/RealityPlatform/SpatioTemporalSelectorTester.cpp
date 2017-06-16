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
    json.capacity();

    auto regionOfInterest = new bvector<GeoPoint2d>();
    regionOfInterest->emplace_back(GeoPoint2d::From(0, 0));
    regionOfInterest->emplace_back(GeoPoint2d::From(20, 0));
    regionOfInterest->emplace_back(GeoPoint2d::From(20, 20));
    regionOfInterest->emplace_back(GeoPoint2d::From(0, 20));
    regionOfInterest->emplace_back(GeoPoint2d::From(0, 0));

    // Low only return low, medium return medium+low and high return all.
    // they are based on the statistics of the whole set.

    auto Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Low, DateCriteria::UpToDate);
    EXPECT_EQ(Ids.size(), 3);

    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Medium, DateCriteria::UpToDate);
    EXPECT_EQ(Ids.size(), 6);

    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::High, DateCriteria::UpToDate);
    EXPECT_EQ(Ids.size(), 9);
    }

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalSelectorFixture, GetIdsFromJsonDateCriteria)
    {
    auto json = RealityModFrameworkTestsUtils::GetJson(L"TestData\\RealityPlatform\\SpatialEntitiesBigList.json");

    auto regionOfInterest = new bvector<GeoPoint2d>();
    regionOfInterest->emplace_back(GeoPoint2d::From(0, 0));
    regionOfInterest->emplace_back(GeoPoint2d::From(20, 0));
    regionOfInterest->emplace_back(GeoPoint2d::From(20, 20));
    regionOfInterest->emplace_back(GeoPoint2d::From(0, 20));
    regionOfInterest->emplace_back(GeoPoint2d::From(0, 0));

    DateTime date;

    //******** Low only
    // past to present
    auto Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Low, DateCriteria::Old);
    EXPECT_EQ(Ids[0], "2002");
    EXPECT_EQ(Ids[1], "2005");
    EXPECT_EQ(Ids[2], "2007");

    // present to past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Low, DateCriteria::UpToDate);
    EXPECT_EQ(Ids[0], "2007");
    EXPECT_EQ(Ids[1], "2005");
    EXPECT_EQ(Ids[2], "2002");

    // mid -> present -> Past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Low, DateCriteria::Recent);
    EXPECT_EQ(Ids[0], "2005");
    EXPECT_EQ(Ids[1], "2007");
    EXPECT_EQ(Ids[2], "2002");

    // ****** Mid + low
    // past to present
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Medium, DateCriteria::Old);
    // mid resolution first
    EXPECT_EQ(Ids[0], "2003");
    EXPECT_EQ(Ids[1], "2006");
    EXPECT_EQ(Ids[2], "2009");
    // low resolution second
    EXPECT_EQ(Ids[3], "2002");
    EXPECT_EQ(Ids[4], "2005");
    EXPECT_EQ(Ids[5], "2007");

    // present to past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Medium, DateCriteria::UpToDate);
    // mid resolution first
    EXPECT_EQ(Ids[0], "2009");
    EXPECT_EQ(Ids[1], "2006");
    EXPECT_EQ(Ids[2], "2003");
    // low resolution second
    EXPECT_EQ(Ids[3], "2007");
    EXPECT_EQ(Ids[4], "2005");
    EXPECT_EQ(Ids[5], "2002");

    // mid -> present -> Past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::Medium, DateCriteria::Recent);
    // mid resolution first
    EXPECT_EQ(Ids[0], "2006");
    EXPECT_EQ(Ids[1], "2009");
    EXPECT_EQ(Ids[2], "2003");
    // low resolution second
    EXPECT_EQ(Ids[3], "2005");
    EXPECT_EQ(Ids[4], "2007");
    EXPECT_EQ(Ids[5], "2002");

    // ****** High + Mid + low
    // past to present
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::High, DateCriteria::Old);
    // High resolution first
    EXPECT_EQ(Ids[0], "2001");
    EXPECT_EQ(Ids[1], "2004");
    EXPECT_EQ(Ids[2], "2008");
    // mid resolution first
    EXPECT_EQ(Ids[3], "2003");
    EXPECT_EQ(Ids[4], "2006");
    EXPECT_EQ(Ids[5], "2009");
    // low resolution second
    EXPECT_EQ(Ids[6], "2002");
    EXPECT_EQ(Ids[7], "2005");
    EXPECT_EQ(Ids[8], "2007");

    // present to past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::High, DateCriteria::UpToDate);
    // High resolution first
    EXPECT_EQ(Ids[0], "2008");
    EXPECT_EQ(Ids[1], "2004");
    EXPECT_EQ(Ids[2], "2001");
    // mid resolution first
    EXPECT_EQ(Ids[3], "2009");
    EXPECT_EQ(Ids[4], "2006");
    EXPECT_EQ(Ids[5], "2003");
    // low resolution second
    EXPECT_EQ(Ids[6], "2007");
    EXPECT_EQ(Ids[7], "2005");
    EXPECT_EQ(Ids[8], "2002");

    // mid -> present -> Past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *regionOfInterest, ResolutionCriteria::High, DateCriteria::Recent);
    // High resolution first
    EXPECT_EQ(Ids[0], "2004");
    EXPECT_EQ(Ids[1], "2008");
    EXPECT_EQ(Ids[2], "2001");
    // mid resolution first
    EXPECT_EQ(Ids[3], "2006");
    EXPECT_EQ(Ids[4], "2009");
    EXPECT_EQ(Ids[5], "2003");
    // low resolution second
    EXPECT_EQ(Ids[6], "2005");
    EXPECT_EQ(Ids[7], "2007");
    EXPECT_EQ(Ids[8], "2002");
    }