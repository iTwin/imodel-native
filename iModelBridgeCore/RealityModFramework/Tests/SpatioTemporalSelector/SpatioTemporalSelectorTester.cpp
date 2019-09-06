/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/BeTest.h>
#include <SpatioTemporalSelector/SpatioTemporalSelector.h>
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

    static void SetUpTestCase()
        {
        s_regionOfInterest = new bvector<GeoPoint2d>();
        s_regionOfInterest->emplace_back(GeoPoint2d::From(0, 0));
        s_regionOfInterest->emplace_back(GeoPoint2d::From(100, 0));
        s_regionOfInterest->emplace_back(GeoPoint2d::From(100, 100));
        s_regionOfInterest->emplace_back(GeoPoint2d::From(0, 100));
        s_regionOfInterest->emplace_back(GeoPoint2d::From(0, 0));
        }
    static void TearDownTestCase()
        {
        delete s_regionOfInterest;
        s_regionOfInterest = nullptr;
        }

    static bvector<GeoPoint2d>* s_regionOfInterest;
};

bvector<GeoPoint2d>* SpatioTemporalSelectorFixture::s_regionOfInterest = nullptr;

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalSelectorFixture, GetIdsFromJsonAll)
    {
    auto json = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\SpatialEntitiesBigList.json");

    auto Ids = SpatioTemporalSelector::GetIDsByResFromJson(json.c_str(), *s_regionOfInterest);
    EXPECT_EQ(Ids.size(), 3);
    EXPECT_EQ(Ids[ResolutionCriteria::Low].size(), 3);
    EXPECT_EQ(Ids[ResolutionCriteria::Medium].size(), 6);
    EXPECT_EQ(Ids[ResolutionCriteria::High].size(), 9);
    }

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalSelectorFixture, OutsideRegionOfInterest)
    {
    auto json = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\SpatialEntitiesBigList.json");

    auto regionOfInterest = bvector<GeoPoint2d>();
    regionOfInterest.emplace_back(GeoPoint2d::From(0, 0));
    regionOfInterest.emplace_back(GeoPoint2d::From(-100, 0));
    regionOfInterest.emplace_back(GeoPoint2d::From(-100, -100));
    regionOfInterest.emplace_back(GeoPoint2d::From(0, -100));
    regionOfInterest.emplace_back(GeoPoint2d::From(0, 0));

    auto Ids = SpatioTemporalSelector::GetIDsByResFromJson(json.c_str(), regionOfInterest);
    EXPECT_EQ(Ids.size(), 3);
    EXPECT_EQ(Ids[ResolutionCriteria::Low].size(), 0);
    EXPECT_EQ(Ids[ResolutionCriteria::Medium].size(), 0);
    EXPECT_EQ(Ids[ResolutionCriteria::High].size(), 0);
    }

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalSelectorFixture, GetIdsByResDistanceFilteringHasNoDataValue)
    {
    auto json = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\SpatialEntitiesMultipleDataforEachCriteria.json");
    auto dataset = SpatialEntityDataset::CreateFromJson(json.c_str());

    //distance filtering is only active if hasNoDataValue is true
    auto Ids = SpatioTemporalSelector::GetIDsByRes(*dataset, *s_regionOfInterest, true);
    EXPECT_EQ(Ids.size(), 3);
    EXPECT_EQ(Ids[ResolutionCriteria::Low].size(), 6);

    EXPECT_EQ(Ids[ResolutionCriteria::Medium].size(), 12);
    EXPECT_EQ(Ids[ResolutionCriteria::High].size(), 18);
    // 2008-2 is the most recent, closest to the center and a high res dataset
    // we expect it to be first
    EXPECT_EQ(Ids[ResolutionCriteria::High][0], "2008-2");

    // Expect 2008 to be just after since its the same data, except further from the center
    EXPECT_EQ(Ids[ResolutionCriteria::High][1], "2008");

    //2002 is the oldest, farther from the center and a los res dataset
    // we expect it to be last
    EXPECT_EQ(Ids[ResolutionCriteria::High][17], "2002");
    }

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalSelectorFixture, GetIdsFromJsonResolutionCriteria)
    {
    auto json = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\SpatialEntitiesBigList.json");

    auto regionOfInterest = bvector<GeoPoint2d>();
    regionOfInterest.emplace_back(GeoPoint2d::From(0, 0));
    regionOfInterest.emplace_back(GeoPoint2d::From(20, 0));
    regionOfInterest.emplace_back(GeoPoint2d::From(20, 20));
    regionOfInterest.emplace_back(GeoPoint2d::From(0, 20));
    regionOfInterest.emplace_back(GeoPoint2d::From(0, 0));

    // Low only return low, medium return medium+low and high return all.
    // they are based on the statistics of the whole set.

    auto Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), regionOfInterest, ResolutionCriteria::Low, DateCriteria::UpToDate);
    EXPECT_EQ(Ids.size(), 3);

    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), regionOfInterest, ResolutionCriteria::Medium, DateCriteria::UpToDate);
    EXPECT_EQ(Ids.size(), 6);

    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), regionOfInterest, ResolutionCriteria::High, DateCriteria::UpToDate);
    EXPECT_EQ(Ids.size(), 9);
    }

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalSelectorFixture, GetIdsFromJsonCriteriaImagery)
    {
    auto json = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\SpatialEntitiesBigList.json");

    //******** Low only
    // past to present
    auto Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Low, DateCriteria::Old);
    EXPECT_EQ(Ids[0], "2002");
    EXPECT_EQ(Ids[1], "2005");
    EXPECT_EQ(Ids[2], "2007");

    // present to past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Low, DateCriteria::UpToDate);
    EXPECT_EQ(Ids[0], "2007");
    EXPECT_EQ(Ids[1], "2005");
    EXPECT_EQ(Ids[2], "2002");

    // mid -> present -> Past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Low, DateCriteria::Recent);
    EXPECT_EQ(Ids[0], "2005");
    EXPECT_EQ(Ids[1], "2007");
    EXPECT_EQ(Ids[2], "2002");

    // ****** Mid + low
    // past to present
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Medium, DateCriteria::Old);
    // mid resolution first
    EXPECT_EQ(Ids[0], "2003");
    EXPECT_EQ(Ids[1], "2006");
    EXPECT_EQ(Ids[2], "2009");
    // low resolution second
    EXPECT_EQ(Ids[3], "2002");
    EXPECT_EQ(Ids[4], "2005");
    EXPECT_EQ(Ids[5], "2007");

    // present to past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Medium, DateCriteria::UpToDate);
    // mid resolution first
    EXPECT_EQ(Ids[0], "2009");
    EXPECT_EQ(Ids[1], "2006");
    EXPECT_EQ(Ids[2], "2003");
    // low resolution second
    EXPECT_EQ(Ids[3], "2007");
    EXPECT_EQ(Ids[4], "2005");
    EXPECT_EQ(Ids[5], "2002");

    // mid -> present -> Past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Medium, DateCriteria::Recent);
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
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::High, DateCriteria::Old);
    // High resolution first
    EXPECT_EQ(Ids[0], "2001");
    EXPECT_EQ(Ids[1], "2004");
    EXPECT_EQ(Ids[2], "2008");
    // mid resolution second
    EXPECT_EQ(Ids[3], "2003");
    EXPECT_EQ(Ids[4], "2006");
    EXPECT_EQ(Ids[5], "2009");
    // low resolution third
    EXPECT_EQ(Ids[6], "2002");
    EXPECT_EQ(Ids[7], "2005");
    EXPECT_EQ(Ids[8], "2007");

    // present to past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::High, DateCriteria::UpToDate);
    // High resolution first
    EXPECT_EQ(Ids[0], "2008");
    EXPECT_EQ(Ids[1], "2004");
    EXPECT_EQ(Ids[2], "2001");
    // mid resolution second
    EXPECT_EQ(Ids[3], "2009");
    EXPECT_EQ(Ids[4], "2006");
    EXPECT_EQ(Ids[5], "2003");
    // low resolution third
    EXPECT_EQ(Ids[6], "2007");
    EXPECT_EQ(Ids[7], "2005");
    EXPECT_EQ(Ids[8], "2002");

    // mid -> present -> Past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::High, DateCriteria::Recent);
    // High resolution first
    EXPECT_EQ(Ids[0], "2004");
    EXPECT_EQ(Ids[1], "2008");
    EXPECT_EQ(Ids[2], "2001");
    // mid resolution second
    EXPECT_EQ(Ids[3], "2006");
    EXPECT_EQ(Ids[4], "2009");
    EXPECT_EQ(Ids[5], "2003");
    // low resolution third
    EXPECT_EQ(Ids[6], "2005");
    EXPECT_EQ(Ids[7], "2007");
    EXPECT_EQ(Ids[8], "2002");
    }

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalSelectorFixture, GetIdsFromJsonCriteriaTerrain)
    {
    auto json = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\SpatialEntitiesBigList.json");
    json.ReplaceAll("Imagery", "Terrain");

    //******** Low only
    // past to present
    auto Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Low, DateCriteria::Old, ResolutionCriteria::Low, DateCriteria::Old);
    EXPECT_EQ(Ids[0], "2002");
    EXPECT_EQ(Ids[1], "2005");
    EXPECT_EQ(Ids[2], "2007");

    // present to past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Low, DateCriteria::UpToDate, ResolutionCriteria::Low, DateCriteria::UpToDate);
    EXPECT_EQ(Ids[0], "2007");
    EXPECT_EQ(Ids[1], "2005");
    EXPECT_EQ(Ids[2], "2002");

    // mid -> present -> Past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Low, DateCriteria::Recent, ResolutionCriteria::Low, DateCriteria::Recent);
    EXPECT_EQ(Ids[0], "2005");
    EXPECT_EQ(Ids[1], "2007");
    EXPECT_EQ(Ids[2], "2002");

    // ****** Mid + low
    // past to present
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Medium, DateCriteria::Old, ResolutionCriteria::Medium, DateCriteria::Old);
    // mid resolution first
    EXPECT_EQ(Ids[0], "2003");
    EXPECT_EQ(Ids[1], "2006");
    EXPECT_EQ(Ids[2], "2009");
    // low resolution second
    EXPECT_EQ(Ids[3], "2002");
    EXPECT_EQ(Ids[4], "2005");
    EXPECT_EQ(Ids[5], "2007");

    // present to past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Medium, DateCriteria::UpToDate, ResolutionCriteria::Medium, DateCriteria::UpToDate);
    // mid resolution first
    EXPECT_EQ(Ids[0], "2009");
    EXPECT_EQ(Ids[1], "2006");
    EXPECT_EQ(Ids[2], "2003");
    // low resolution second
    EXPECT_EQ(Ids[3], "2007");
    EXPECT_EQ(Ids[4], "2005");
    EXPECT_EQ(Ids[5], "2002");

    // mid -> present -> Past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::Medium, DateCriteria::Recent, ResolutionCriteria::Medium, DateCriteria::Recent);
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
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::High, DateCriteria::Old, ResolutionCriteria::High, DateCriteria::Old);
    // High resolution first
    EXPECT_EQ(Ids[0], "2001");
    EXPECT_EQ(Ids[1], "2004");
    EXPECT_EQ(Ids[2], "2008");
    // mid resolution second
    EXPECT_EQ(Ids[3], "2003");
    EXPECT_EQ(Ids[4], "2006");
    EXPECT_EQ(Ids[5], "2009");
    // low resolution third
    EXPECT_EQ(Ids[6], "2002");
    EXPECT_EQ(Ids[7], "2005");
    EXPECT_EQ(Ids[8], "2007");

    // present to past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::High, DateCriteria::UpToDate, ResolutionCriteria::High, DateCriteria::UpToDate);
    // High resolution first
    EXPECT_EQ(Ids[0], "2008");
    EXPECT_EQ(Ids[1], "2004");
    EXPECT_EQ(Ids[2], "2001");
    // mid resolution second
    EXPECT_EQ(Ids[3], "2009");
    EXPECT_EQ(Ids[4], "2006");
    EXPECT_EQ(Ids[5], "2003");
    // low resolution third
    EXPECT_EQ(Ids[6], "2007");
    EXPECT_EQ(Ids[7], "2005");
    EXPECT_EQ(Ids[8], "2002");

    // mid -> present -> Past
    Ids = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, ResolutionCriteria::High, DateCriteria::Recent, ResolutionCriteria::High, DateCriteria::Recent);
    // High resolution first
    EXPECT_EQ(Ids[0], "2004");
    EXPECT_EQ(Ids[1], "2008");
    EXPECT_EQ(Ids[2], "2001");
    // mid resolution second
    EXPECT_EQ(Ids[3], "2006");
    EXPECT_EQ(Ids[4], "2009");
    EXPECT_EQ(Ids[5], "2003");
    // low resolution third
    EXPECT_EQ(Ids[6], "2005");
    EXPECT_EQ(Ids[7], "2007");
    EXPECT_EQ(Ids[8], "2002");
    }

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalSelectorFixture, GetIDsFromJsonCriteriaResolution)
    {
    auto json = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\SpatialEntitiesBigList.json");

    DateTime dateMin;
    DateTime::FromString(dateMin, "2000-01-01");

    DateTime dateMax;
    DateTime::FromString(dateMax, "2017-01-01");


    auto vector = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, 0.1, 5.0, dateMin, dateMax) ;
    EXPECT_EQ(vector.size(), 3);
    EXPECT_EQ(vector[0], "2001");
    EXPECT_EQ(vector[1], "2004");
    EXPECT_EQ(vector[2], "2008");
    }

//=====================================================================================
//! @bsimethod                                  Remi.Charbonneau              06/2017
//=====================================================================================
TEST_F(SpatioTemporalSelectorFixture, GetIDsFromJsonCriteriaDate)
    {
    auto json = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\SpatialEntitiesBigList.json");

    DateTime dateMin;
    DateTime::FromString(dateMin, "2000-01-01");

    DateTime dateMax;
    DateTime::FromString(dateMax, "2005-01-01");


    auto vector = SpatioTemporalSelector::GetIDsFromJson(json.c_str(), *s_regionOfInterest, 0.1, 50, dateMin, dateMax);
    EXPECT_EQ(vector.size(), 4);
    EXPECT_EQ(vector[0], "2001");
    EXPECT_EQ(vector[1], "2002");
    EXPECT_EQ(vector[2], "2003");
    EXPECT_EQ(vector[3], "2004");
    }