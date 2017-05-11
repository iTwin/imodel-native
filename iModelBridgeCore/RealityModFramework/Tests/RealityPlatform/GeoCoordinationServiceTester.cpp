#include <Bentley/BeTest.h>
#include <RealityPlatform/GeoCoordinationService.h>
#include <RealityPlatform/RealityDataService.h>
#include <ostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::Return;
using ::testing::_;

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
TEST(GeoCoordinationService, ShouldSetServerComponentsCorrectly)
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
struct MockWSGRequest : WSGRequest, testing::WithParamInterface<WSGRequest_response_state>
    {
    MockWSGRequest() : WSGRequest()
        {}

    //MOCK_CONST_METHOD5(_PerformRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
    MOCK_CONST_METHOD5(PerformAzureRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
    MOCK_CONST_METHOD4(PrepareRequest, CURL*(const WSGURL& wsgRequest, RawServerResponse& responseString, bool verifyPeer, BeFile* file));

    void _PerformRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer = true, BeFile* file = nullptr, bool retry = true) const override
        {
        response.status = (RequestStatus) GetParam().status;
        response.body = GetParam().body;
        }

    void PerformRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer = true, BeFile* file = nullptr, bool retry = true) const override
        {
        response.status = (RequestStatus) GetParam().status;
        response.body = GetParam().body;
        }
    };

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
class GeoCoordinationServiceRequestFixture : public testing::Test
    {
    public:
        DummyServiceRequest* dummyServiceRequest;
        MockWSGRequest* dummyWSGRequest;

        GeoCoordinationServiceRequestFixture()
            {
            dummyServiceRequest = new DummyServiceRequest;
            dummyWSGRequest = new MockWSGRequest;
            }

        ~GeoCoordinationServiceRequestFixture()
            {
            delete dummyServiceRequest;
            delete dummyWSGRequest;
            }

        void errorCallback(Utf8String basicMessage, const RawServerResponse& rawResponse)
            {
            dummyServiceRequest->errorCallback(basicMessage, rawResponse);
            }
    };


//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct GeoCoordinateServiceBasic : GeoCoordinationServiceRequestFixture, testing::WithParamInterface<WSGRequest_response_state>
    {
    GeoCoordinateServiceBasic()
        {}

    };


//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_P(GeoCoordinateServiceBasic, BasicRequest)
    {
    GeoCoordinationService serviceUnderTest = GeoCoordinationService();

    serviceUnderTest.SetServerComponents("https://dev-contextservices.bentley.com/", "2.4", "IndexECPlugin-Server", "RealityModeling");
    auto dummyRequest = new DummyServiceRequest;
    auto response = GeoCoordinationService::BasicRequest(dummyRequest, GetParam().keyword);
    ASSERT_TRUE(response.status == GetParam().result);

    }

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
TEST_P(GeoCoordinateServiceBasic, BasicPagedRequest)
    {
    GeoCoordinationService serviceUnderTest = GeoCoordinationService();

    serviceUnderTest.SetServerComponents("https://dev-contextservices.bentley.com/", "2.4", "IndexECPlugin-Server", "RealityModeling");
    auto dummyRequest = new SpatialEntityWithDetailsSpatialRequest(bvector<GeoPoint2d>(), 0);
    auto response = GeoCoordinationService::BasicPagedRequest(dummyRequest, GetParam().keyword);
    ASSERT_TRUE(response.status == GetParam().result);

    }

//=====================================================================================
//! @bsimethod                                 Remi.Charbonneau              05/2017
//=====================================================================================
INSTANTIATE_TEST_CASE_P(Default, GeoCoordinateServiceBasic, testing::Values(
    WSGRequest_response_state {CURLE_OK           , "{ \"memberPresent\": \"Yes\" }" , "memberPresent"    , RequestStatus::OK},
    WSGRequest_response_state {CURLE_LOGIN_DENIED , "{ \"memberPresent\": \"Yes\" }" , "memberPresent"    , RequestStatus::BADREQ},
    WSGRequest_response_state {CURLE_OK           , "INVALIDJSON"                    , "memberPresent"    , RequestStatus::BADREQ},
    WSGRequest_response_state {CURLE_OK           , "{ \"memberPresent\": \"Yes\" }" ,  "memberNotPresent" , RequestStatus::BADREQ},
    WSGRequest_response_state {CURLE_OK           , "{ \"errorMessage\": \"Yes\" }"  ,   "memberNotPresent" , RequestStatus::BADREQ}
));