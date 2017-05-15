#include <Bentley/BeTest.h>
#include <RealityPlatform/GeoCoordinationService.h>
#include <RealityPlatform/RealityDataService.h>
#include <ostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::Return;
using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;

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

struct ErrorClass
    {
    MOCK_CONST_METHOD2(errorCallBack, void(Utf8String basicMessage, const RawServerResponse& rawResponse));

    ErrorClass()
        {

        }
    };

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
class GeoCoordinationServiceRequestFixture : public testing::Test
    {
    public:
        DummyServicePagedRequest* dummyPagedRequest;
        GeoCoordinationService* geoCoordinateServiceUnderTest;
        static ErrorClass* errorClass;

        GeoCoordinationServiceRequestFixture()
            {

            dummyPagedRequest = new DummyServicePagedRequest();
            geoCoordinateServiceUnderTest = new GeoCoordinationService();
            errorClass = new ErrorClass();
            geoCoordinateServiceUnderTest->SetErrorCallback(GeoCoordinationServiceRequestFixture::mockErrorCallBack);
            }

        ~GeoCoordinationServiceRequestFixture()
            {

            delete dummyPagedRequest;
            delete geoCoordinateServiceUnderTest;
            delete errorClass;
            }


        static void mockErrorCallBack(Utf8String basicMessage, const RawServerResponse& rawResponse)
            {
            if (errorClass != nullptr)
                {
                errorClass->errorCallBack(basicMessage, rawResponse);
                }
            }
    };

ErrorClass* GeoCoordinationServiceRequestFixture::errorClass = nullptr;

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct GeoCoordinateServiceBasicRequest : GeoCoordinationServiceRequestFixture, testing::WithParamInterface<WSGRequest_response_state>
    {
    public:
        MockWSGRequest* dummyWSGRequest;
        GeoCoordinateServiceBasicRequest()
            {
            dummyWSGRequest = new MockWSGRequest;
            }
        ~GeoCoordinateServiceBasicRequest()
            {
            delete dummyWSGRequest;
            }
    };


//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_P(GeoCoordinateServiceBasicRequest, BasicRequest)
    {
    auto dummyRequest = new DummyServiceRequest;

    ON_CALL(*(this->dummyWSGRequest), PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([&] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.status = static_cast<RequestStatus>(GetParam().status);
        response.body = GetParam().body;
        }));

    auto response = GeoCoordinationService::BasicRequest(dummyRequest, GetParam().keyword);
    ASSERT_TRUE(response.status == GetParam().result);

    }

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_P(GeoCoordinateServiceBasicRequest, BasicPagedRequest)
    {
    auto dummyRequest = SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>(), 0);

    ON_CALL(*(this->dummyWSGRequest), PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([&] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
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

    ON_CALL(*(this->dummyWSGRequest), PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([&] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
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
        }

    auto response = GeoCoordinationService::BasicPagedRequest(dummyPagedRequest, GetParam().keyword);
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

    EXPECT_CALL(*(this->errorClass), errorCallBack(Eq("DownloadReport File not found"), _));

    geoCoordinateServiceUnderTest->SetErrorCallback(GeoCoordinationServiceRequestFixture::mockErrorCallBack);
    auto rawResponse = RawServerResponse();
    GeoCoordinationService::Request(request, rawResponse);
    }

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceRequestFixture, DownloadReportBadRequest)
    {
    auto filename = BeFileName("e:\\temp\\exist.txt");
    auto request = DownloadReportUploadRequest("randomguid", "myidentifier", filename);

    EXPECT_CALL(*(this->errorClass), errorCallBack(Eq("Error Uploading DownloadReport"), _));

    geoCoordinateServiceUnderTest->SetErrorCallback(GeoCoordinationServiceRequestFixture::mockErrorCallBack);
    auto rawResponse = RawServerResponse();
    GeoCoordinationService::Request(request, rawResponse);
    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);
    }

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct GeoCoordinationServiceAdvanceRequest : GeoCoordinationServiceRequestFixture
    {
    public:
        MockWSGRequest* dummyWSGRequest;
        GeoCoordinationServiceAdvanceRequest()
            :GeoCoordinationServiceRequestFixture()
            {
            dummyWSGRequest = new MockWSGRequest;
            }
        ~GeoCoordinationServiceAdvanceRequest()
            {
            delete dummyWSGRequest;
            }
    };

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceAdvanceRequest, PreparedPackageRequestFailedCreation)
    {
    EXPECT_CALL(*(this->errorClass), errorCallBack(Eq("PreparedPackageRequest failed to create file at provided location"), _));

    auto filename = BeFileName("mybadfile:\\myfile.txt");
    auto request = PreparedPackageRequest("myidentifier");
    auto rawResponse = RawServerResponse();

    GeoCoordinationService::Request(request, filename, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::UNSENT);
    }

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceAdvanceRequest, PreparedPackageRequestBadRequest)
    {

    EXPECT_CALL(*(this->errorClass), errorCallBack(Eq("Package download failed with response"), _));
    ON_CALL(*(this->dummyWSGRequest), PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.curlCode = CURLE_FILE_COULDNT_READ_FILE;
        }));

    auto filename = BeFileName("E:\\temp\\myfile.txt");
    auto request = PreparedPackageRequest("myidentifier");
    auto rawResponse = RawServerResponse();

    GeoCoordinationService::Request(request, filename, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::BADREQ);
    }


//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceAdvanceRequest, PreparedPackageRequestGoodRequest)
    {

    EXPECT_CALL(*(this->errorClass), errorCallBack(_, _)).Times(0);
    ON_CALL(*(this->dummyWSGRequest), PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.curlCode = CURLE_OK;
        }));

    auto filename = BeFileName("E:\\temp\\myfile.txt");
    auto request = PreparedPackageRequest("myidentifier");
    auto rawResponse = RawServerResponse();

    GeoCoordinationService::Request(request, filename, rawResponse);

    EXPECT_EQ(rawResponse.status, RequestStatus::OK);
    }

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_F(GeoCoordinationServiceAdvanceRequest, PackagePreparationRequestGoodRequest)
    {

    EXPECT_CALL(*(this->errorClass), errorCallBack(_, _)).Times(0);
    ON_CALL(*(this->dummyWSGRequest), PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
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
TEST_F(GeoCoordinationServiceAdvanceRequest, PackagePreparationRequestBadRequest)
    {

    EXPECT_CALL(*(this->errorClass), errorCallBack(_, _)).Times(0);
    ON_CALL(*(this->dummyWSGRequest), PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.curlCode = CURLE_OK;
        response.status = RequestStatus::UNSENT;
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
TEST_F(GeoCoordinationServiceAdvanceRequest, SpatialEntityWithDetailsGoodRequestLastPage)
    {

    EXPECT_CALL(*(this->errorClass), errorCallBack(_, _)).Times(0);
    ON_CALL(*(this->dummyWSGRequest), PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
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
TEST_F(GeoCoordinationServiceAdvanceRequest, SpatialEntityWithDetailsGoodRequestNotLastPage)
    {

    EXPECT_CALL(*(this->errorClass), errorCallBack(_, _)).Times(0);
    ON_CALL(*(this->dummyWSGRequest), PerformRequest(_, _, _, _, _)).WillByDefault(Invoke([] (const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
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
        else if(request->GetStartIndex() == 1)
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

