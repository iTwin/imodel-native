//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#include <Bentley/BeTest.h>
#include <RealityPlatformTools/GeoCoordinationService.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <ostream>
#include <Bentley/BeTextFile.h>
#include <Bentley/BeFileName.h>
#include "../Common/RealityModFrameworkTestsCommon.h"

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
    MOCK_CONST_METHOD0(GetUserAgent, Utf8StringCR());
    MOCK_METHOD2(errorCallback, void(Utf8String basicMessage, const RawServerResponse& rawResponse));
    MOCK_CONST_METHOD0(_PrepareHttpRequestStringAndPayload, void());

    DummyServiceRequest()
        :GeoCoordinationServiceRequest()
        {
        m_serverName = GeoCoordinationService::GetServerName();
        m_userAgent = GeoCoordinationService::GetUserAgent();
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
    MOCK_CONST_METHOD0(GetUserAgent, Utf8StringCR());
    MOCK_METHOD2(errorCallback, void(Utf8String basicMessage, const RawServerResponse& rawResponse));
    MOCK_CONST_METHOD0(_PrepareHttpRequestStringAndPayload, void());
    MOCK_CONST_METHOD0(AdvancePage, void());

    DummyServicePagedRequest()
        :GeoCoordinationServicePagedRequest()
        {
        m_serverName = GeoCoordinationService::GetServerName();
        m_userAgent = GeoCoordinationService::GetUserAgent();
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
            << "toolCode: " << obj.status
            << " body: " << obj.body
            << " keyword: " << obj.keyword
            << " result: " << obj.result;
        }
    };

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST(GeoCoordinationService, SetServerComponentsCorrectly)
    {
    auto serviceUnderTest = GeoCoordinationService();
    serviceUnderTest.SetServerComponents("Server", "WSGProtocol", "RepoName", "SchemaName");
    serviceUnderTest.SetUserAgent("Some User Agent");
    ASSERT_STREQ(serviceUnderTest.GetServerName().c_str(), "Server");
    ASSERT_STREQ(serviceUnderTest.GetWSGProtocol().c_str(), "WSGProtocol");
    ASSERT_STREQ(serviceUnderTest.GetRepoName().c_str(), "RepoName");
    ASSERT_STREQ(serviceUnderTest.GetSchemaName().c_str(), "SchemaName");
    ASSERT_STREQ(serviceUnderTest.GetUserAgent().c_str(), "Some User Agent");
    }

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//! bb
//! Fixture class for most of the GeoCoordinationService request.
//! It hold a Mock WSGRequest so that all call to WSGRequest::GetInstance() can be intercepted
//=====================================================================================
class GeoCoordinationServiceRequestFixture : public MockGeoCoordinationServiceFixture
    {
public:
    //
    WCharCP GetDirectory()
        {
        BeFileName outDir;
        BeTest::GetHost().GetTempDir(outDir);
        outDir.AppendToPath(L"GeoCoordinationServiceTester");
        return outDir;
        }

    void InitTestDirectory(WCharCP directoryname)
        {
        if (BeFileName::DoesPathExist(directoryname))
            BeFileName::EmptyAndRemoveDirectory(directoryname);
        BeFileName::CreateNewDirectory(directoryname);
        }
    };

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

    s_geoCoordinateService->SetErrorCallback(GeoCoordinationServiceRequestFixture::mockErrorCallBack);
    auto rawResponse = RawServerResponse();
    GeoCoordinationService::Request(request, rawResponse);
    }

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, DownloadReportBadRequest)
    {
    WString directory(GetDirectory());
    directory.append(L"/DownloadReportBadRequest");
    InitTestDirectory(directory.c_str());
    WString fileName(directory);
    fileName.append(L"/xmlTestfile.xml");
    BeFileStatus status;
    BeTextFilePtr tempFile = BeTextFile::Open(status, fileName.c_str(), TextFileOpenType::Write, TextFileOptions::None);
    tempFile->Close();

    auto request = DownloadReportUploadRequest("randomguid", "myidentifier", BeFileName(fileName));

    EXPECT_EQ(request.GetMessageSize(), 0);

    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("Error Uploading DownloadReport"), _));

    s_geoCoordinateService->SetErrorCallback(GeoCoordinationServiceRequestFixture::mockErrorCallBack);
    auto rawResponse = RawServerResponse();
    GeoCoordinationService::Request(request, rawResponse);
    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);

    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
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
TEST_F(GeoCoordinationServiceRequestFixture, PreparedPackageRequestBadRequest)
    {

    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("Package download failed with response"), _));
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.toolCode = CURLE_FILE_COULDNT_READ_FILE;
        }));

    WString directory(GetDirectory());
    directory.append(L"/PreparedPackageRequestBadRequest");
    InitTestDirectory(directory.c_str());
    WString fileName(directory);
    fileName.append(L"/tempFile.tmp");
    BeFileStatus status;
    BeTextFilePtr tempFile = BeTextFile::Open(status, fileName.c_str(), TextFileOpenType::Write, TextFileOptions::None);
    tempFile->Close();


    auto request = PreparedPackageRequest("myidentifier");
    auto rawResponse = RawServerResponse();

    GeoCoordinationService::Request(request, BeFileName(fileName), rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);

    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    }


//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, PreparedPackageRequestGoodRequest)
    {

    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.toolCode = CURLE_OK;
        }));

    WString directory(GetDirectory());
    directory.append(L"/PreparedPackageRequestGoodRequest");
    InitTestDirectory(directory.c_str());
    WString fileName(directory);
    fileName.append(L"/tempFile.tmp");
    BeFileStatus status;
    BeTextFilePtr tempFile = BeTextFile::Open(status, fileName.c_str(), TextFileOpenType::Write, TextFileOptions::None);
    tempFile->Close();

    auto request = PreparedPackageRequest("myidentifier");
    auto rawResponse = RawServerResponse();

    GeoCoordinationService::Request(request, BeFileName(fileName), rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::OK);

    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    }


//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, PackagePreparationRequestGood)
    {

    EXPECT_CALL(*s_errorClass, errorCallBack(_, _)).Times(0);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.toolCode = CURLE_OK;
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
        response.toolCode = CURLE_OK;
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
        response.toolCode = CURLE_OK;
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

    auto request = SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>(), RealityDataBase::Classification::IMAGERY);
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
        response.toolCode = CURLE_OK;
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


    auto request = SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>(), RealityDataBase::Classification::IMAGERY);
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
    EXPECT_CALL(*s_errorClass, errorCallBack(Eq("Tool error"), _)).Times(1);
    ON_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.toolCode = CURLE_OK;
        response.status = RequestStatus::BADREQ;
        }));

    auto request = SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>(), RealityDataBase::Classification::IMAGERY);
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
        response.toolCode = CURLE_OK;
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
        response.toolCode = CURLE_OK;
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
        response.toolCode = CURLE_OK;
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
        response.toolCode = CURLE_OK;
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
        response.toolCode = CURLE_OK;
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
    EXPECT_STREQ(spatialEntitiesDataSource->GetUri().ToString().c_str(), "http://www.example.com");
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
        response.toolCode = CURLE_OK;
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
        response.toolCode = CURLE_OK;
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
        response.toolCode = CURLE_OK;
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
        response.toolCode = CURLE_OK;
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
        response.toolCode = CURLE_OK;
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