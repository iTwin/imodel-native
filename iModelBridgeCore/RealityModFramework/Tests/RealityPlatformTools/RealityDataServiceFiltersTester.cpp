//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <RealityPlatform/RealityDataObjects.h>
#include <ostream>

using testing::ValuesIn;

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

	auto expectedFootprint = R"(polygon={"Coordinates": [{"Longitude": "-92.000000000", "Latitude": "39.000000000"},{"Longitude": "-92.000000000", "Latitude": "38.000000000"},{"Longitude": "-93.000000000", "Latitude": "38.000000000"},{"Longitude": "-93.000000000", "Latitude": "39.000000000"},{"Longitude": "-92.000000000", "Latitude": "39.000000000"}]})";
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
//! @bsimethod                                  Spencer.Mason                  12/2017
//=====================================================================================
TEST(RealityDataFilterCreator, FilterByAccessDate)
    {
    DateTime minDate(DateTime::Kind::Utc, 2000, 01, 01, 12, 00, 25, 200);
    DateTime maxDate(DateTime::Kind::Utc, 2016, 12, 31, 23, 59, 59, 999);

    RDSFilter filter = RealityDataFilterCreator::FilterByAccessDate(minDate, maxDate);
    EXPECT_STREQ(filter.ToString().c_str(), "LastAccessedTimestamp+ge+'2000-01-01T12:00:25.200Z'+and+LastAccessedTimestamp+le+'2016-12-31T23:59:59.999Z'");
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
	EXPECT_STREQ(filter.ToString().c_str(), "RelatedId+eq+'MyProjectID'");
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
	EXPECT_STREQ(filter.ToString().c_str(), "RealityDataId+eq+'MyRealityDataID'+and+RelatedId+eq+'MyProjectID'");
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
	EXPECT_STREQ(filter.ToString().c_str(), "RealityDataId+eq+'MyRealityDataID'+or+RelatedId+eq+'MyProjectID'");
	}

//=====================================================================================
//! @bsiclass                                  Remi.Charbonneau              06/2017
//=====================================================================================
struct RealityMap
    {
    friend std::ostream& operator<<(std::ostream& os, const RealityMap& obj)
    {
        return os
            << "field: " << static_cast<int>(obj.field)
            << " name: " << obj.name;
    }

    RealityDataField field;
    Utf8String name;

    RealityMap(RealityDataField field_, Utf8String name_) : field(field_), name(name_)
        {}

    static bvector<RealityMap> GetAllDataField()
        {
        bvector<RealityMap> realityDataFieldVector{};

        realityDataFieldVector.emplace_back(RealityDataField::Id, "Id");
        realityDataFieldVector.emplace_back(RealityDataField::OrganizationId, "OrganizationId");
        realityDataFieldVector.emplace_back(RealityDataField::ContainerName, "ContainerName");
        realityDataFieldVector.emplace_back(RealityDataField::DataLocationGuid, "DataLocationGuid");
        realityDataFieldVector.emplace_back(RealityDataField::Name, "Name");
        realityDataFieldVector.emplace_back(RealityDataField::Dataset, "Dataset");
        realityDataFieldVector.emplace_back(RealityDataField::Description, "Description");
        realityDataFieldVector.emplace_back(RealityDataField::RootDocument, "RootDocument");
        realityDataFieldVector.emplace_back(RealityDataField::Size, "Size");
        realityDataFieldVector.emplace_back(RealityDataField::SizeUpToDate, "SizeUpToDate");
        realityDataFieldVector.emplace_back(RealityDataField::Classification, "Classification");
        realityDataFieldVector.emplace_back(RealityDataField::Type, "Type");
        realityDataFieldVector.emplace_back(RealityDataField::Streamed, "Streamed");
        realityDataFieldVector.emplace_back(RealityDataField::Footprint, "Footprint");
        realityDataFieldVector.emplace_back(RealityDataField::ThumbnailDocument, "ThumbnailDocument");
        realityDataFieldVector.emplace_back(RealityDataField::MetadataUrl, "MetadataUrl");
        realityDataFieldVector.emplace_back(RealityDataField::Copyright, "Copyright");
        realityDataFieldVector.emplace_back(RealityDataField::TermsOfUse, "TermsOfUse");
        realityDataFieldVector.emplace_back(RealityDataField::ResolutionInMeters, "ResolutionInMeters");
        realityDataFieldVector.emplace_back(RealityDataField::AccuracyInMeters, "AccuracyInMeters");
        realityDataFieldVector.emplace_back(RealityDataField::Visibility, "Visibility");
        realityDataFieldVector.emplace_back(RealityDataField::Listable, "Listable");
        realityDataFieldVector.emplace_back(RealityDataField::CreatedTimestamp, "CreatedTimestamp");
        realityDataFieldVector.emplace_back(RealityDataField::ModifiedTimestamp, "ModifiedTimestamp");
        realityDataFieldVector.emplace_back(RealityDataField::LastAccessedTimestamp, "LastAccessedTimestamp");
        realityDataFieldVector.emplace_back(RealityDataField::OwnedBy, "OwnedBy");
        realityDataFieldVector.emplace_back(RealityDataField::Group, "Group");

        return realityDataFieldVector;
        }
    };

//=====================================================================================
//! @bsiclass                                  Remi.Charbonneau              06/2017
//=====================================================================================
class RealityDataFieldFixture : public ::testing::TestWithParam<RealityMap> 
    {

    };

//=====================================================================================
//! @bsimethod                                Remi.Charbonneau              06/2017
//=====================================================================================
TEST_P(RealityDataFieldFixture, SortBy)
    {
    RealityDataPagedRequest request {};
    request.SortBy(GetParam().field, true);

    auto requestString = request.GetHttpRequestString();
    Utf8String searchedPortion = Utf8PrintfString("$orderby=%s",GetParam().name.c_str());
    EXPECT_TRUE(requestString.Contains(searchedPortion));
    }
 

INSTANTIATE_TEST_CASE_P(Default,
                        RealityDataFieldFixture,
                        ValuesIn(RealityMap::GetAllDataField()));