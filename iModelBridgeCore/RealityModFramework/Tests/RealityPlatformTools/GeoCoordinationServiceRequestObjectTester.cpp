//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include <RealityPlatformTools/GeoCoordinationService.h>
#include "../Common/RealityModFrameworkTestsCommon.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::Invoke;
using ::testing::_;
using ::testing::HasSubstr;

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct SpatialEntityWithDetailsSpatialRequest_response_state
{
    Utf8String filter;
    Utf8String body;
    Utf8String keyword;
    RequestStatus result;

    friend std::ostream& operator<<(std::ostream& os, const SpatialEntityWithDetailsSpatialRequest_response_state& obj)
    {
        return os
            << "toolCode: " /*<< obj.status
            << " body: " << obj.body
            << " keyword: " << obj.keyword
            << " result: " << obj.result*/;
    }
};


//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//! GeoCoordinationServiceRequestsFixture
//! Fixture class for most of the GeoCoordinationService request objects.
//! It hold a Mock WSGRequest so that all call to WSGRequest::GetInstance() can be intercepted
//=====================================================================================
class GeoCoordinationServiceRequestsFixture : public MockGeoCoordinationServiceFixture
{
public:
    static Utf8String GetClassificationString(int classification)
    {
        Utf8String classificationString("&");
        if (classification > 0)
        {
            classificationString.append("Classification+in+[");
            if (classification & RealityDataBase::Classification::IMAGERY)
                classificationString.append("'Imagery',");
            if (classification & RealityDataBase::Classification::TERRAIN)
                classificationString.append("'Terrain',");
            if (classification & RealityDataBase::Classification::MODEL)
                classificationString.append("'Model',");
            if (classification & RealityDataBase::Classification::PINNED)
                classificationString.append("'Pinned',");
            classificationString = classificationString.substr(0, classificationString.size() - 1); //remove comma
            classificationString.append("]");
        }
        return classificationString;
    }

    static Utf8String GetAllCoordinationFieldSortString(GeoCoordinationField field, bool ascending)
    {
        Utf8String order("&$orderby=");

        switch (field)
        {
        case GeoCoordinationField::Id:
            order.append("Id");
            break;
        case GeoCoordinationField::Footprint:
            order.append("Footprint");
            break;
        case GeoCoordinationField::Name:
            order.append("Name");
            break;
        case GeoCoordinationField::Description:
            order.append("Description");
            break;
        case GeoCoordinationField::ContactInformation:
            order.append("ContactInformation");
            break;
        case GeoCoordinationField::Keywords:
            order.append("Keywords");
            break;
        case GeoCoordinationField::Legal:
            order.append("Legal");
            break;
        case GeoCoordinationField::TermsOfUse:
            order.append("TermsOfUse");
            break;
        case GeoCoordinationField::DataSourceType:
            order.append("DataSourceType");
            break;
        case GeoCoordinationField::AccuracyInMeters:
            order.append("AccuracyInMeters");
            break;
        case GeoCoordinationField::Date:
            order.append("Date");
            break;
        case GeoCoordinationField::Classification:
            order.append("Classification");
            break;
        case GeoCoordinationField::FileSize:
            order.append("FileSize");
            break;
        case GeoCoordinationField::Streamed:
            order.append("Streamed");
            break;
        case GeoCoordinationField::SpatialDataSourceId:
            order.append("SpatialDataSourceId");
            break;
        case GeoCoordinationField::ResolutionInMeters:
            order.append("ResolutionInMeters");
            break;
        case GeoCoordinationField::ThumbnailURL:
            order.append("ThumbnailURL");
            break;
        case GeoCoordinationField::DataProvider:
            order.append("DataProvider");
            break;
        case GeoCoordinationField::DataProviderName:
            order.append("DataProviderName");
            break;
        case GeoCoordinationField::Dataset:
            order.append("Dataset");
            break;
        case GeoCoordinationField::Occlusion:
            order.append("Occlusion");
            break;
        case GeoCoordinationField::MetadataURL:
            order.append("MetadataURL");
            break;
        case GeoCoordinationField::RawMetadataURL:
            order.append("RawMetadataURL");
            break;
        case GeoCoordinationField::RawMetadataFormat:
            order.append("RawMetadataFormat");
            break;
        case GeoCoordinationField::SubAPI:
            order.append("SubAPI");
            break;
        }

        if (ascending)
            order.append("+asc");
        else
            order.append("+desc");

        return order;
    }

    static bvector<GeoCoordinationField> GetAllCoordinationField()
    {
        bvector<GeoCoordinationField> vectorField;
        for (auto i = static_cast<int>(GeoCoordinationField::Id); i <= static_cast<int>(GeoCoordinationField::SubAPI); i++)
        {
            vectorField.push_back(static_cast<GeoCoordinationField>(i));
        }

        return vectorField;
    }
};

typedef ::std::tr1::tuple<int, Utf8String, GeoCoordinationField, bool> SpatialEntityWithDetails;

struct SpatialEntityWithDetailsSpatialRequestFixture : GeoCoordinationServiceRequestsFixture, ::testing::WithParamInterface<SpatialEntityWithDetails>
{
    SpatialEntityWithDetailsSpatialRequestFixture()
    {}
};

TEST_P(SpatialEntityWithDetailsSpatialRequestFixture, SpatialEntityWithDetailsSpatialRequestDefaultConstructor)
{
    auto requestUnderTest = SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>(), ::std::tr1::get<0>(GetParam()));

    requestUnderTest.SetFilter(::std::tr1::get<1>(GetParam()));
    Utf8String filterString("&$filter=");


    requestUnderTest.SortBy(::std::tr1::get<2>(GetParam()), ::std::tr1::get<3>(GetParam()));

    auto requestString = requestUnderTest.GetHttpRequestString();

    //"example.com", "99", "Dummy-Server", "VirtualModeling");
    Utf8String expectedRequestString = "https://example.com/v99/Repositories/Dummy-Server/VirtualModeling/SpatialEntityWithDetailsView?polygon={points:[],coordinate_system:'4326'}";
    expectedRequestString.append("&$filter=");
    expectedRequestString.append(::std::tr1::get<1>(GetParam()));
    expectedRequestString.append(GetClassificationString(::std::tr1::get<0>(GetParam())));
    expectedRequestString.append(GetAllCoordinationFieldSortString(::std::tr1::get<2>(GetParam()), ::std::tr1::get<3>(GetParam())));
    expectedRequestString.append("&$skip=0&$top=25");
    EXPECT_STREQ(requestString.c_str(), expectedRequestString.c_str());

}

INSTANTIATE_TEST_CASE_P(Classification, SpatialEntityWithDetailsSpatialRequestFixture,
    ::testing::Combine(testing::Range(1, 9),
    testing::Values(L"Id+eq+'1'"),
    testing::Values(GeoCoordinationField::RawMetadataFormat),
    testing::Values(true, false)));

INSTANTIATE_TEST_CASE_P(CoordinationField, SpatialEntityWithDetailsSpatialRequestFixture,
    ::testing::Combine(testing::Range(1, 2),
    testing::Values(L"Id+eq+'1'"),
    testing::ValuesIn(GeoCoordinationServiceRequestsFixture::GetAllCoordinationField()),
    testing::Values(true, false)));

TEST_F(GeoCoordinationServiceRequestsFixture, SpatialEntityWithDetailsSpatialRequestNavigationTest)
{
    auto requestUnderTest = SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>());

    //default page is 25 and start at 0
    Utf8String subStringToVerify = Utf8PrintfString("&$skip=%u&$top=%u", 0, 25);
    EXPECT_THAT(requestUnderTest.GetHttpRequestString().c_str(), HasSubstr(subStringToVerify.c_str()));

    requestUnderTest.AdvancePage();
    requestUnderTest.AdvancePage();
    subStringToVerify = Utf8PrintfString("&$skip=%u&$top=%u", 50, 25);
    EXPECT_THAT(requestUnderTest.GetHttpRequestString().c_str(), HasSubstr(subStringToVerify.c_str()));

    requestUnderTest.RewindPage();
    subStringToVerify = Utf8PrintfString("&$skip=%u&$top=%u", 25, 25);
    EXPECT_THAT(requestUnderTest.GetHttpRequestString().c_str(), HasSubstr(subStringToVerify.c_str()));

    requestUnderTest.SetPageSize(50);
    EXPECT_EQ(requestUnderTest.GetPageSize(), 50);
    subStringToVerify = Utf8PrintfString("&$skip=%u&$top=%u", 25, 50);
    EXPECT_THAT(requestUnderTest.GetHttpRequestString().c_str(), HasSubstr(subStringToVerify.c_str()));

    requestUnderTest.GoToPage(3);
    subStringToVerify = Utf8PrintfString("&$skip=%u&$top=%u", 50 * 3, 50);
    EXPECT_THAT(requestUnderTest.GetHttpRequestString().c_str(), HasSubstr(subStringToVerify.c_str()));

}


TEST_F(GeoCoordinationServiceRequestsFixture, SpatialEntityWithDetailsSpatialRequestPolygonTest)
{

    auto pointVector = bvector<GeoPoint2d>({{0,0},{0,1},{1,1},{1,0},{0,0}});


    auto requestUnderTest = SpatialEntityWithDetailsSpatialRequest(pointVector);

    auto requestString = requestUnderTest.GetHttpRequestString();

    EXPECT_THAT(requestString.c_str(), HasSubstr("polygon={points:[[0.000000,0.000000],[0.000000,1.000000],[1.000000,1.000000],[1.000000,0.000000],[0.000000,0.000000]],coordinate_system:'4326'}"));
}

TEST_F(GeoCoordinationServiceRequestsFixture, SpatialEntityWithDetailsByIdRequestPrepareRequest)
{
    auto requestUnderTest = SpatialEntityWithDetailsByIdRequest("MyIdentifier1");

    auto requestString = requestUnderTest.GetHttpRequestString();

    ////"example.com", "99", "Dummy-Server", "VirtualModeling");
    EXPECT_STREQ(requestString.c_str(), "https://example.com/v99/Repositories/Dummy-Server/VirtualModeling/SpatialEntityWithDetailsView/MyIdentifier1");
}

TEST_F(GeoCoordinationServiceRequestsFixture, SpatialEntityByIdRequestRequestPrepareRequest)
{
    auto requestUnderTest = SpatialEntityByIdRequest("MyIdentifier1");

    auto requestString = requestUnderTest.GetHttpRequestString();

    ////"example.com", "99", "Dummy-Server", "VirtualModeling");
    EXPECT_STREQ(requestString.c_str(), "https://example.com/v99/Repositories/Dummy-Server/VirtualModeling/SpatialEntity/MyIdentifier1");
}

TEST_F(GeoCoordinationServiceRequestsFixture, SpatialEntityDataSourceByIdRequestPrepareRequest)
{
    auto requestUnderTest = SpatialEntityDataSourceByIdRequest("MyIdentifier1");

    auto requestString = requestUnderTest.GetHttpRequestString();

    ////"example.com", "99", "Dummy-Server", "VirtualModeling");
    EXPECT_STREQ(requestString.c_str(), "https://example.com/v99/Repositories/Dummy-Server/VirtualModeling/SpatialDataSource/MyIdentifier1");
}

TEST_F(GeoCoordinationServiceRequestsFixture, SpatialEntityServerByIdRequestPrepareRequest)
{
    auto requestUnderTest = SpatialEntityServerByIdRequest("MyIdentifier1");

    auto requestString = requestUnderTest.GetHttpRequestString();

    ////"example.com", "99", "Dummy-Server", "VirtualModeling");
    EXPECT_STREQ(requestString.c_str(), "https://example.com/v99/Repositories/Dummy-Server/VirtualModeling/Server/MyIdentifier1");
}

TEST_F(GeoCoordinationServiceRequestsFixture, SpatialEntityMetadataByIdRequestPrepareRequest)
{
    auto requestUnderTest = SpatialEntityMetadataByIdRequest("MyIdentifier1");

    auto requestString = requestUnderTest.GetHttpRequestString();

    ////"example.com", "99", "Dummy-Server", "VirtualModeling");
    EXPECT_STREQ(requestString.c_str(), "https://example.com/v99/Repositories/Dummy-Server/VirtualModeling/Metadata/MyIdentifier1");
}

TEST_F(GeoCoordinationServiceRequestsFixture, PackagePreparationRequestPrepareRequest)
{
    auto pointVector = bvector<GeoPoint2d>({{0,0}, {0,1}, {1,1}, {1,0}, {0,0}});
    auto pointStringVector = bvector<Utf8String>({{"Id1"},{"Id2"}});
    auto requestUnderTest = PackagePreparationRequest(pointVector, pointStringVector);

    auto requestString = requestUnderTest.GetHttpRequestString();
    auto requestPayload = requestUnderTest.GetRequestPayload();
    auto requestHeaders = requestUnderTest.GetRequestHeaders();

    ////"example.com", "99", "Dummy-Server", "VirtualModeling");
    EXPECT_STREQ(requestString.c_str(), "https://example.com/v99/Repositories/Dummy-Server/VirtualModeling/PackageRequest/");
    EXPECT_TRUE(requestHeaders.Contains("Content-Type: application/json"));

    Utf8String expectedPayload = R"({"instance":{"instanceId":null,"className":"PackageRequest","schemaName":"RealityModeling","properties":{"RequestedEntities":[)";
    expectedPayload.append(R"({ "Id":"Id1"},{ "Id":"Id2"})");
    expectedPayload.append(R"(],"CoordinateSystem":null,"OSM": false,"Polygon":"[)");
    expectedPayload.append(R"([0.000000,0.000000],[0.000000,1.000000],[1.000000,1.000000],[1.000000,0.000000],[0.000000,0.000000])");
    expectedPayload.append(R"(]"}}, "requestOptions":{"CustomOptions":{"Version":"2", "Requestor":"GeoCoordinationService", "RequestorVersion":"1.0" }}})");

    EXPECT_STREQ(requestPayload.c_str(), expectedPayload.c_str());
}

TEST_F(GeoCoordinationServiceRequestsFixture, PreparedPackageRequestPrepareRequest)
{
    auto requestUnderTest = PreparedPackageRequest("MyIdentifier1");

    auto requestString = requestUnderTest.GetHttpRequestString();

    ////"example.com", "99", "Dummy-Server", "VirtualModeling");
    EXPECT_STREQ(requestString.c_str(), "https://example.com/v99/Repositories/Dummy-Server/VirtualModeling/PreparedPackage/MyIdentifier1/$file");

}

TEST_F(GeoCoordinationServiceRequestsFixture, DownloadReportUploadRequestPrepareRequest)
{
    auto requestUnderTest = DownloadReportUploadRequest("{82e6361d-23f3-4022-90c9-784cc47cb3d3}","MyIdentifier",BeFileName("myFile.txt"));

    auto requestString = requestUnderTest.GetHttpRequestString();

    ////"example.com", "99", "Dummy-Server", "VirtualModeling");
    EXPECT_STREQ(requestString.c_str(), "https://example.com/v99/Repositories/Dummy-Server/VirtualModeling/DownloadReport/{82e6361d-23f3-4022-90c9-784cc47cb3d3}/$file");

    auto requestHeaders = requestUnderTest.GetRequestHeaders();
    EXPECT_TRUE(requestHeaders.Contains("Content-Disposition : attachment; filename=\"MyIdentifier\""));
}


TEST_F(GeoCoordinationServiceRequestsFixture, GeoCoordinationServiceRequestGetter)
{
    auto requestUnderTest = SpatialEntityWithDetailsByIdRequest("SuperUniqueID");

    ////"example.com", "99", "Dummy-Server", "VirtualModeling");
    EXPECT_STREQ(requestUnderTest.GetServerName().c_str(), "example.com");
    EXPECT_STREQ(requestUnderTest.GetVersion().c_str(), "99");
    EXPECT_STREQ(requestUnderTest.GetSchema().c_str(), "VirtualModeling");
    EXPECT_STREQ(requestUnderTest.GetRepoId().c_str(), "Dummy-Server");
    EXPECT_STREQ(requestUnderTest.GetUserAgent().c_str(), "MockGeoCoordinationServiceFixture - dummy user agent");
}

TEST_F(GeoCoordinationServiceRequestsFixture, GeoCoordinationServicePagedRequestGetter)
{
    auto requestUnderTest = SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>());

    ////"example.com", "99", "Dummy-Server", "VirtualModeling");
    EXPECT_STREQ(requestUnderTest.GetServerName().c_str(), "example.com");
    EXPECT_STREQ(requestUnderTest.GetVersion().c_str(), "99");
    EXPECT_STREQ(requestUnderTest.GetSchema().c_str(), "VirtualModeling");
    EXPECT_STREQ(requestUnderTest.GetRepoId().c_str(), "Dummy-Server");
    EXPECT_STREQ(requestUnderTest.GetUserAgent().c_str(), "MockGeoCoordinationServiceFixture - dummy user agent");

}