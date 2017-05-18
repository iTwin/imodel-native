//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/GeoCoordinationServiceTester.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <Bentley/BeTest.h>
#include <RealityPlatform/GeoCoordinationService.h>
#include <RealityPlatform/RealityDataService.h>
#include <ostream>
#include <Bentley/BeTextFile.h>
#include <Bentley/BeFileName.h>
#include <Bentley/Desktop/FileSystem.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::Return;
using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::NiceMock;

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
class DummyServiceRequest : public GeoCoordinationServiceRequest
{
public:
    MOCK_CONST_METHOD0(GetServerName, Utf8StringCR());
    MOCK_CONST_METHOD0(GetVersion, Utf8StringCR());
    MOCK_CONST_METHOD0(GetSchema, Utf8StringCR());
    MOCK_CONST_METHOD0(GetRepoId, Utf8StringCR());
    MOCK_METHOD2(errorCallback, void(Utf8String basicMessage, const RawServerResponse& rawResponse));
    MOCK_CONST_METHOD0(_PrepareHttpRequestStringAndPayload, void());

    DummyServiceRequest()
        :GeoCoordinationServiceRequest()
    {
        m_serverName = GeoCoordinationService::GetServerName();
        m_validRequestString = true;
        m_httpRequestString.append("/test/test.aspx");

    }
};

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
class DummyServicePagedRequest : public GeoCoordinationServicePagedRequest
{
public:
    MOCK_CONST_METHOD0(GetServerName, Utf8StringCR());
    MOCK_CONST_METHOD0(GetVersion, Utf8StringCR());
    MOCK_CONST_METHOD0(GetSchema, Utf8StringCR());
    MOCK_CONST_METHOD0(GetRepoId, Utf8StringCR());
    MOCK_METHOD2(errorCallback, void(Utf8String basicMessage, const RawServerResponse& rawResponse));
    MOCK_CONST_METHOD0(_PrepareHttpRequestStringAndPayload, void());
    MOCK_CONST_METHOD0(AdvancePage, void());

    DummyServicePagedRequest()
        :GeoCoordinationServicePagedRequest()
    {
        m_serverName = GeoCoordinationService::GetServerName();
        m_validRequestString = true;
        m_httpRequestString.append("/test/test.aspx");

    }
};

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct WSGRequest_response_state
{
    int status;
    Utf8String body;
    Utf8String keyword;
    RequestStatus result;

    friend std::ostream& operator<<(std::ostream& os, const WSGRequest_response_state& obj)
    {
        return os
            << "curlCode: " << obj.status
            << " body: " << obj.body
            << " keyword: " << obj.keyword
            << " result: " << obj.result;
    }
};

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct MockWSGRequest : WSGRequest
{
    MockWSGRequest() : WSGRequest()
    {}

    MOCK_CONST_METHOD5(PerformAzureRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
    MOCK_CONST_METHOD4(PrepareRequest, CURL*(const WSGURL& wsgRequest, RawServerResponse& responseString, bool verifyPeer, BeFile* file));
    MOCK_CONST_METHOD5(_PerformRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
    MOCK_CONST_METHOD5(PerformRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
};

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct ErrorClass
{
    MOCK_CONST_METHOD2(errorCallBack, void(Utf8String basicMessage, const RawServerResponse& rawResponse));

    ErrorClass()
    {

    }
};

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST(GeoCoordinationService, SetServerComponentsCorrectly)
{
    auto serviceUnderTest = GeoCoordinationService();
    serviceUnderTest.SetServerComponents("Server", "WSGProtocol", "RepoName", "SchemaName");
    ASSERT_STREQ(serviceUnderTest.GetServerName().c_str(), "Server");
    ASSERT_STREQ(serviceUnderTest.GetWSGProtocol().c_str(), "WSGProtocol");
    ASSERT_STREQ(serviceUnderTest.GetRepoName().c_str(), "RepoName");
    ASSERT_STREQ(serviceUnderTest.GetSchemaName().c_str(), "SchemaName");
}

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//! GeoCoordinationServiceRequestFixture
//! Fixture class for most of the GeoCoordinationService request.
//! It hold a Mock WSGRequest so that all call to WSGRequest::GetInstance() can be intercepted
//=====================================================================================
class GeoCoordinationServiceRequestFixture : public testing::Test
{
public:
    static GeoCoordinationService* s_geoCoordinateServiceUnderTest;
    static ErrorClass* s_errorClass;
    static NiceMock<MockWSGRequest>* s_mockWSGInstance;

    static void SetUpTestCase()
    {
        s_geoCoordinateServiceUnderTest = new GeoCoordinationService();
        s_errorClass = new ErrorClass();
        s_geoCoordinateServiceUnderTest->SetErrorCallback(GeoCoordinationServiceRequestFixture::mockErrorCallBack);
        s_mockWSGInstance = new NiceMock<MockWSGRequest>();
    }

    static void TearDownTestCase()
    {
        delete s_geoCoordinateServiceUnderTest;
        delete s_errorClass;
        delete s_mockWSGInstance;
        s_geoCoordinateServiceUnderTest = nullptr;
        s_errorClass = nullptr;
        s_mockWSGInstance = nullptr;
    }

    static void mockErrorCallBack(Utf8String basicMessage, const RawServerResponse& rawResponse)
    {
        if (s_errorClass != nullptr)
        {
            s_errorClass->errorCallBack(basicMessage, rawResponse);
        }
    }
};

ErrorClass* GeoCoordinationServiceRequestFixture::s_errorClass = nullptr;
GeoCoordinationService* GeoCoordinationServiceRequestFixture::s_geoCoordinateServiceUnderTest = nullptr;
NiceMock<MockWSGRequest>* GeoCoordinationServiceRequestFixture::s_mockWSGInstance = nullptr;

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct GeoCoordinateServiceBasicRequest : GeoCoordinationServiceRequestFixture, testing::WithParamInterface<WSGRequest_response_state>
{

};



//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_P(GeoCoordinateServiceBasicRequest, BasicRequest)
{
    auto dummyRequest = new DummyServiceRequest;

    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([&] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.status = static_cast<RequestStatus>(GetParam().status);
        response.body = GetParam().body;
    }));

    auto response = GeoCoordinationService::BasicRequest(dummyRequest, GetParam().keyword);
    ASSERT_TRUE(response.status == GetParam().result);

    delete dummyRequest;
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_P(GeoCoordinateServiceBasicRequest, BasicPagedRequest)
{
    auto dummyRequest = SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>(), 0);

    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([&] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.status = static_cast<RequestStatus>(GetParam().status);
        response.body = GetParam().body;
    }));

    auto response = GeoCoordinationService::BasicPagedRequest(&dummyRequest, GetParam().keyword);
    ASSERT_TRUE(response.status == GetParam().result);
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_P(GeoCoordinateServiceBasicRequest, AdvancePageIsCalledAfterSuccessfulRequest)
{

    auto dummyPagedRequest = new DummyServicePagedRequest();

    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([&] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.status = static_cast<RequestStatus>(GetParam().status);
        response.body = GetParam().body;
    }));

    if (GetParam().result == RequestStatus::OK)
    {
        EXPECT_CALL(*dummyPagedRequest, AdvancePage());
    }
    else
    {
        EXPECT_CALL(*dummyPagedRequest, AdvancePage()).Times(0);
        EXPECT_CALL(*s_errorClass, errorCallBack(_, _));
    }

    auto response = GeoCoordinationService::BasicPagedRequest(dummyPagedRequest, GetParam().keyword);

    delete dummyPagedRequest;
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
INSTANTIATE_TEST_CASE_P(BadRequest, GeoCoordinateServiceBasicRequest, testing::Values(
    WSGRequest_response_state {CURLE_LOGIN_DENIED , "{ \"memberPresent\": \"Yes\" }" , "memberPresent"    , RequestStatus::BADREQ},
    WSGRequest_response_state {CURLE_OK           , "INVALIDJSON"                    , "memberPresent"    , RequestStatus::BADREQ},
    WSGRequest_response_state {CURLE_OK           , "{ \"memberPresent\": \"Yes\" }" ,  "memberNotPresent" , RequestStatus::BADREQ},
    WSGRequest_response_state {CURLE_OK           , "{ \"errorMessage\": \"Yes\" }"  ,   "memberNotPresent" , RequestStatus::BADREQ}
));

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
INSTANTIATE_TEST_CASE_P(GoodRequest, GeoCoordinateServiceBasicRequest, testing::Values(
    WSGRequest_response_state {CURLE_OK           , "{ \"memberPresent\": \"Yes\" }" , "memberPresent"    , RequestStatus::OK}
));

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, DownloadRequestFileNotfoundCallback)
{
    auto filename = BeFileName("afilenimethatdontexist.idontexist");
    auto request = DownloadReportUploadRequest("randomguid", "myidentifier", filename);

    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("DownloadReport File not found"), _));

    s_geoCoordinateServiceUnderTest->SetErrorCallback(GeoCoordinationServiceRequestFixture::mockErrorCallBack);
    auto rawResponse = RawServerResponse();
    GeoCoordinationService::Request(request, rawResponse);
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, DownloadReportBadRequest)
{
    BeFileName filename;
    Desktop::FileSystem::BeGetTempFileName(filename, BeFileName(""), L"myPrefix");
    auto request = DownloadReportUploadRequest("randomguid", "myidentifier", filename);

    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("Error Uploading DownloadReport"), _));

    s_geoCoordinateServiceUnderTest->SetErrorCallback(GeoCoordinationServiceRequestFixture::mockErrorCallBack);
    auto rawResponse = RawServerResponse();
    GeoCoordinationService::Request(request, rawResponse);
    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);

    //delete dummy file
    BeFileName::BeDeleteFile(filename);
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, PreparedPackageRequestFailedCreation)
{
    EXPECT_CALL(*(this->s_errorClass), errorCallBack(Eq("PreparedPackageRequest failed to create file at provided location"), _));

    auto filename = BeFileName("??????");
    auto request = PreparedPackageRequest("myidentifier");
    auto rawResponse = RawServerResponse();

    GeoCoordinationService::Request(request, filename, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::UNSENT);
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, PreparedPackageRequestRequestBad)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("Package download failed with response"), _));
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_FILE_COULDNT_READ_FILE;
    }));

    BeFileName filename;
    if (Desktop::FileSystem::BeGetTempFileName(filename, BeFileName(""), L"myPrefix") == BeFileNameStatus::Success)
        {

        auto request = PreparedPackageRequest("myidentifier");
        auto rawResponse = RawServerResponse();

        GeoCoordinationService::Request(request, filename, rawResponse);

        EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);

        BeFileName::BeDeleteFile(filename);
        }
    else
        {
        BeAssert("Can't create temp file");
        }
}


//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, PreparedPackageRequestRequestGood)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_OK;
    }));

    BeFileName filename;
    if (Desktop::FileSystem::BeGetTempFileName(filename, BeFileName(""), L"myPrefix") == BeFileNameStatus::Success)
        {

        auto request = PreparedPackageRequest("myidentifier");
        auto rawResponse = RawServerResponse();

        GeoCoordinationService::Request(request, filename, rawResponse);

        EXPECT_EQ(rawResponse.status, RequestStatus::OK);

        BeFileName::BeDeleteFile(filename);
        }
    else
        {
        BeAssert("Can't create temp file");
        }
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, PackagePreparationRequestGood)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"({
            "changedInstance": {
            "instanceAfterChange":
                {
            "instanceId": "MyID"
                }
            }
            })";
    }));

    auto request = PackagePreparationRequest(bvector<GeoPoint2d>(), bvector<Utf8String>());
    auto rawResponse = RawServerResponse();

    auto packageId = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_STREQ(packageId.c_str(), "MyID");
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, PackagePreparationRequestBad)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(1);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::BADREQ;
        response.body = R"(
            {
                "changedInstance": 
                    {
                    "instanceAfterChange":
                        {
                        "instanceId": "MyID"
                        }
                    }
            })";
    }));

    auto request = PackagePreparationRequest(bvector<GeoPoint2d>(), bvector<Utf8String>());
    auto rawResponse = RawServerResponse();

    auto packageId = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_STREQ(packageId.c_str(), "");
    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);
}


//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityWithDetailsRequestGoodLastPage)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"(
            {
            "instances":   
                [
                    { 
                        "properties" : 
                            {
                            "Name" : "Name1",
                            "Classification": "Imagery"
                            }
                    },
                    {
                        "properties" : 
                            {
                            "Name" : "Name2",
                            "Classification": "Imagery"
                            }
                    }
                ]
            })";
    }));

    auto request = SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>(), Classification::Imagery);
    auto rawResponse = RawServerResponse();

    auto spatialEntities = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_STREQ(spatialEntities[0]->GetName().c_str(), "Name1");
    EXPECT_STREQ(spatialEntities[1]->GetName().c_str(), "Name2");
    EXPECT_EQ(rawResponse.status, RequestStatus::LASTPAGE);
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityWithDetailsRequestNotLastPage)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        auto request = static_cast<const SpatialEntityWithDetailsSpatialRequest*>(&wsgRequest);
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;

        if (request->GetStartIndex() == 0)
        {
            response.body = R"(
                {
                "instances":   
                    [
                        { 
                            "properties" : 
                                {
                                "Name" : "Name1"
                                }
                        }
                    ]
                })";
        }
        else if (request->GetStartIndex() == 1)
        {
            response.body = R"(
                {
                "instances":   
                    [
                        { 
                            "properties" : 
                                {
                                "Name" : "Name2"
                                }
                        }
                    ]
                })";
        }
        else
        {
            response.body = R"(
                {
                "instances":   
                    [
                    ]
                })";
        }
    }));


    auto request = SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>(), Classification::Imagery);
    auto rawResponse = RawServerResponse();

    // request only 1 page so that we can request the next one.
    request.SetPageSize(1);

    auto spatialEntities = GeoCoordinationService::Request(request, rawResponse);
    EXPECT_STREQ(spatialEntities[0]->GetName().c_str(), "Name1");
    EXPECT_EQ(rawResponse.status, RequestStatus::OK);

    auto spatialEntities2 = GeoCoordinationService::Request(request, rawResponse);
    EXPECT_STREQ(spatialEntities2[0]->GetName().c_str(), "Name2");
    EXPECT_EQ(rawResponse.status, RequestStatus::OK);

    auto spatialEntities3 = GeoCoordinationService::Request(request, rawResponse);
    EXPECT_EQ(spatialEntities3.size(), 0);
    EXPECT_EQ(rawResponse.status, RequestStatus::LASTPAGE);
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityWithDetailsRequestBad)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("SpatialEntityWithDetailsSpatialRequest failed with response"), _)).Times(1);
    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("Curl error"), _)).Times(1);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::BADREQ;
    }));

    auto request = SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>(), Classification::Imagery);
    auto rawResponse = RawServerResponse();

    auto spatialEntities = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityMetadataByIdRequestGood)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"(
                {
                "instances":   
                    [
                        { 
                            "properties" : 
                                {
                                "Id" : "MyIdentifier",
                                "Description" : "MyDescription"
                                }
                        }
                    ]
                })";
    }));

    auto request = SpatialEntityMetadataByIdRequest("MyIdentifier");
    auto rawResponse = RawServerResponse();

    auto spatialEntitiesMetadata = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::OK);
    EXPECT_STREQ(spatialEntitiesMetadata->GetId().c_str(), "MyIdentifier");
    EXPECT_STREQ(spatialEntitiesMetadata->GetDescription().c_str(), "MyDescription");
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityMetadataByIdRequestBad)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("SpatialEntityMetadataByIdRequest failed with response"), _)).Times(1);
    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("Error message in the JSON"), _)).Times(1);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"(
                {
                "errorMessage":   "My Message"
                })";
    }));

    auto request = SpatialEntityMetadataByIdRequest("MyIdentifier");
    auto rawResponse = RawServerResponse();

    auto spatialEntitiesMetadata = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);
}


//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityServerByIdRequestGood)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"(
                {
                "instances":   
                    [
                        { 
                            "properties" : 
                                {
                                "Id" : "MyIdentifier",
                                "CommunicationProtocol" : "MyCommunicationProtocol"
                                }
                        }
                    ]
                })";
    }));

    auto request = SpatialEntityServerByIdRequest("MyIdentifier");
    auto rawResponse = RawServerResponse();

    auto spatialEntitiesServer = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::OK);
    EXPECT_STREQ(spatialEntitiesServer->GetId().c_str(), "MyIdentifier");
    EXPECT_STREQ(spatialEntitiesServer->GetProtocol().c_str(), "MyCommunicationProtocol");
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityServerByIdRequestBad)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("SpatialEntityServerByIdRequest failed with response"), _)).Times(1);
    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("Error message in the JSON"), _)).Times(1);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"(
                {
                "errorMessage":   "My Message"
                })";
    }));

    auto request = SpatialEntityServerByIdRequest("MyIdentifier");
    auto rawResponse = RawServerResponse();

    auto spatialEntitiesServer = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);
}


//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityDataSourceByIdRequestGood)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"(
                {
                "instances":   
                    [
                        { 
                            "properties" : 
                                {
                                "Id" : "MyIdentifier",
                                "MainURL" : "http://www.example.com"
                                }
                        }
                    ]
                })";
    }));

    auto request = SpatialEntityDataSourceByIdRequest("MyIdentifier");
    auto rawResponse = RawServerResponse();

    auto spatialEntitiesDataSource = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::OK);
    EXPECT_STREQ(spatialEntitiesDataSource->GetId().c_str(), "MyIdentifier");
    EXPECT_STREQ(spatialEntitiesDataSource->GetUrl().c_str(), "http://www.example.com");
}


//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityDataSourceByIdRequestBad)
{

    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("SpatialEntityDataSourceByIdRequest failed with response"), _)).Times(1);
    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("Error message in the JSON"), _)).Times(1);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
    {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"(
                {
                "errorMessage":   "My Message"
                })";
    }));

    auto request = SpatialEntityDataSourceByIdRequest("MyIdentifier");
    auto rawResponse = RawServerResponse();

    auto spatialEntitiesDataSource = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);
}

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityWithDetailsByIdRequestGood)
    {

    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"(
            {
            "instances":   
                [
                    { 
                        "properties" : 
                            {
                            "Id" : "MyIdentifier",
                            "Name": "MyName1"
                            }
                    }
                ]
            })";
        }));

    auto request = SpatialEntityWithDetailsByIdRequest("MyIdentifier");
    auto rawResponse = RawServerResponse();

    auto spatialEntity = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::OK);
    EXPECT_STREQ(spatialEntity->GetIdentifier().c_str(), "MyIdentifier");
    EXPECT_STREQ(spatialEntity->GetName().c_str(), "MyName1");
    }

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityWithDetailsByIdRequestBad)
    {

    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("SpatialEntityRequest failed with response"), _)).Times(1);
    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("Error message in the JSON"), _)).Times(1);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"(
            {
            "errorMessage" : "Myerror"
            })";
        }));

    auto request = SpatialEntityWithDetailsByIdRequest("MyIdentifier");
    auto rawResponse = RawServerResponse();

    auto spatialEntity = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);
    }

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityByIdRequestRequestBad)
    {
    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("SpatialEntityRequest failed with response"), _)).Times(1);
    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("Error message in the JSON"), _)).Times(1);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"(
            {
            "errorMessage" : "Myerror"
            })";
        }));

    auto request = SpatialEntityByIdRequest("MyIdentifier");
    auto rawResponse = RawServerResponse();

    auto spatialEntity = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);
    }

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, SpatialEntityByIdRequestRequestGood)
    {
    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::OK;
        response.body = R"(
            {
            "instances":   
                [
                    { 
                        "properties" : 
                            {
                            "Id" : "MyIdentifier",
                            "Name": "MyName1"
                            }
                    }
                ]
            })";
        }));

    auto request = SpatialEntityByIdRequest("MyIdentifier");
    auto rawResponse = RawServerResponse();

    auto spatialEntity = GeoCoordinationService::Request(request, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::OK);
    EXPECT_STREQ(spatialEntity->GetIdentifier().c_str(), "MyIdentifier");
    EXPECT_STREQ(spatialEntity->GetName().c_str(), "MyName1");
    }