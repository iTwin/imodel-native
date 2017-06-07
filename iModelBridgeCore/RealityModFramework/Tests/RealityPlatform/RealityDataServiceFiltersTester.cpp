//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/RealityDataServiceFiltersTester.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include <RealityPlatform/RealityDataService.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByName)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByName("MyName");
	EXPECT_STREQ(filter.ToString().c_str(), "Name+eq+'MyName'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByClassification)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByClassification(RealityDataBase::Classification::MODEL);
	EXPECT_STREQ(filter.ToString().c_str(), "Classification+eq+'Model'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterBySize)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterBySize(0, UINT64_MAX);
	EXPECT_STREQ(filter.ToString().c_str(), "Size+ge+0+and+Size+le+4294967295");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterSpatial)
	{
	auto footprint = bvector<GeoPoint2d>();
    footprint.emplace_back(GeoPoint2d::From(-92, 39));
    footprint.emplace_back(GeoPoint2d::From(-92, 38));
    footprint.emplace_back(GeoPoint2d::From(-93, 38));
    footprint.emplace_back(GeoPoint2d::From(-93, 39));
    footprint.emplace_back(GeoPoint2d::From(-92, 39));

	auto expectedFootprint = R"(polygon={\"points\":[[-92.000000,39.000000],[-92.000000,38.000000],[-93.000000,38.000000],[-93.000000,39.000000],[-92.000000,39.000000]], \"coordinate_system\":\"1555\"})";
	RDSFilter filter = RealityDataFilterCreator::FilterSpatial(footprint, 1555);
	EXPECT_STREQ(filter.ToString().c_str(), expectedFootprint);
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByOwner)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByOwner("importantOwner@example.com");
	EXPECT_STREQ(filter.ToString().c_str(), "OwnedBy+eq+'importantOwner%40example%2Ecom'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByCreationDate)
	{
	DateTime minDate(DateTime::Kind::Utc, 2000, 01, 01, 12, 00, 25, 200);
	DateTime maxDate(DateTime::Kind::Utc, 2016, 12, 31, 23, 59, 59, 999);

	RDSFilter filter = RealityDataFilterCreator::FilterByCreationDate(minDate, maxDate);
	EXPECT_STREQ(filter.ToString().c_str(), "CreatedTimestamp+ge+'2000-01-01T12:00:25.200Z'+and+CreatedTimestamp+le+'2016-12-31T23:59:59.999Z'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByModificationDate)
	{
	DateTime minDate(DateTime::Kind::Utc, 2000, 01, 01, 12, 00, 25, 200);
	DateTime maxDate(DateTime::Kind::Utc, 2016, 12, 31, 23, 59, 59, 999);

	RDSFilter filter = RealityDataFilterCreator::FilterByModificationDate(minDate, maxDate);
	EXPECT_STREQ(filter.ToString().c_str(), "ModifiedTimestamp+ge+'2000-01-01T12:00:25.200Z'+and+ModifiedTimestamp+le+'2016-12-31T23:59:59.999Z'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByVisibility)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterVisibility(RealityDataBase::Visibility::PERMISSION);
	EXPECT_STREQ(filter.ToString().c_str(), "Visibility+eq+'PERMISSION'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByResolution)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByResolution(0, 200, true);
	EXPECT_STREQ(filter.ToString().c_str(), "ResolutionInMeters+ge+'0.000000'+and+ResolutionInMeters+le+'200.000000'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByAccuracy)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByAccuracy(0, 200, true);
	EXPECT_STREQ(filter.ToString().c_str(), "AccuracyInMeters+ge+'0.000000'+and+AccuracyInMeters+le+'200.000000'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByType)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByType("MyType");
	EXPECT_STREQ(filter.ToString().c_str(), "Type+eq+'MyType'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByDataset)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByDataset("SomeDataset");
	EXPECT_STREQ(filter.ToString().c_str(), "Dataset+eq+'SomeDataset'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByGroup)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterByGroup("YourGroup");
	EXPECT_STREQ(filter.ToString().c_str(), "Group+eq+'YourGroup'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterRelationshipByRealityDataId)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterRelationshipByRealityDataId("MyRealityDataID");
	EXPECT_STREQ(filter.ToString().c_str(), "RealityDataId+eq+'MyRealityDataID'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterRelationshipByProjectId)
	{
	RDSFilter filter = RealityDataFilterCreator::FilterRelationshipByProjectId("MyProjectID");
	EXPECT_STREQ(filter.ToString().c_str(), "ProjectId+eq+'MyProjectID'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, GroupFiltersAND)
	{
	bvector<RDSFilter> filtersVector;
	filtersVector.emplace_back(RealityDataFilterCreator::FilterRelationshipByRealityDataId("MyRealityDataID"));
	filtersVector.emplace_back(RealityDataFilterCreator::FilterRelationshipByProjectId("MyProjectID"));
	auto filter = RealityDataFilterCreator::GroupFiltersAND(filtersVector);
	EXPECT_STREQ(filter.ToString().c_str(), "RealityDataId+eq+'MyRealityDataID'+and+ProjectId+eq+'MyProjectID'");
	}

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
TEST(RealityDataFilterCreator, GroupFiltersOR)
	{
	bvector<RDSFilter> filtersVector;
	filtersVector.emplace_back(RealityDataFilterCreator::FilterRelationshipByRealityDataId("MyRealityDataID"));
	filtersVector.emplace_back(RealityDataFilterCreator::FilterRelationshipByProjectId("MyProjectID"));
	auto filter = RealityDataFilterCreator::GroupFiltersOR(filtersVector);
	EXPECT_STREQ(filter.ToString().c_str(), "RealityDataId+eq+'MyRealityDataID'+or+ProjectId+eq+'MyProjectID'");
	}

