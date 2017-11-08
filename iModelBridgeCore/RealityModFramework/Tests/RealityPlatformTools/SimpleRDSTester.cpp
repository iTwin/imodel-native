//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatformTools/SimpleRDSTester.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include "../../RealityPlatformTools/RealityDataServiceInternal.h"
#include "../Common/RealityModFrameworkTestsCommon.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::NiceMock;
using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::StrEq;
using ::testing::HasSubstr;
using ::testing::Matcher;
using ::testing::Mock;
using ::testing::Return;

/*static void Message(const char* pMsg)
    {
    std::cout << pMsg << std::endl;
    }*/

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
/*TEST_F(SimpleRDSFixture, SetServerComponents)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.header = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatform\\WSGHeader.json");
        }));

    MockRDSRequestManager* mockRDSrm = new MockRDSRequestManager();
    mockRDSrm->SetErrorCallback(Message);
    EXPECT_CALL(*mockRDSrm, MakeBuddiCall());//.WillByDefault(Return("defaultserver.com"));
    mockRDSrm->Init();

    // Test the SetServerComponents set by RDSRequestManager
    EXPECT_STREQ(RealityDataService::GetServerName().c_str(), "defaultserver.com");
    EXPECT_STREQ(RealityDataService::GetWSGProtocol().c_str(), "2.6");
    EXPECT_STREQ(RealityDataService::GetRepoName().c_str(), "S3MXECPlugin--Server");
    EXPECT_STREQ(RealityDataService::GetSchemaName().c_str(), "S3MX");
    EXPECT_TRUE(RealityDataService::AreParametersSet());
    }*/

RealityDataService* SimpleRDSFixture::s_realityDataServices = nullptr;

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataEnterpriseStatTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\EnterpriseStat.json");
        }));

    ConnectedRealityDataEnterpriseStat stat = ConnectedRealityDataEnterpriseStat();
    ConnectedResponse response = stat.GetStats();
    
    EXPECT_TRUE(response.simpleSuccess);
    EXPECT_EQ(stat.GetUltimateId(), "e82a584b-9fae-409f-9581-fd154f7b9ef9");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataProjectRelationshipIdRequestTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship?$filter=RealityDataId+eq+'72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a6'");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\RealityDataRelationship.json");
        }));

    ConnectedRealityDataProjectRelationship rel = ConnectedRealityDataProjectRelationship();
    rel.SetRealityDataId("72adad30-c07c-465d-a1fe-2f2dfac950a6");
    bvector<ConnectedRealityDataProjectRelationshipPtr> results;
    ConnectedResponse response = rel.RetrieveAllForRDId(results);

    EXPECT_TRUE(response.simpleSuccess);
    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0]->GetRealityDataId(), "f4425509-55c4-4e03-932a-d67b87ace30f");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataProjectRelationshipCreateRequestTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship");

        Utf8String expectedPayload = R"({"instance":{"className": "RealityDataRelationship","schemaName":"S3MX", "properties": { "RelationType" : "CONNECT-Project", "RelatedId" : "MYProjectID", "RealityDataId": "MyIdentifier"}}})";

        EXPECT_EQ(wsgRequest.GetRequestType(), WSGURL::HttpRequestType::POST_Request);

        auto payload = wsgRequest.GetRequestPayload();
        EXPECT_STREQ(payload.c_str(), expectedPayload.c_str());

        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        }));

    ConnectedRealityDataProjectRelationship rel = ConnectedRealityDataProjectRelationship();
    rel.SetRealityDataId("MyIdentifier");
    ConnectedResponse response = rel.Create();

    EXPECT_FALSE(response.simpleSuccess);

    rel.SetRelatedId("MYProjectID");
    response = rel.Create();

    EXPECT_TRUE(response.simpleSuccess);
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataDocumentTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
       {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Document/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a7");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;

        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\RealityDataDocument.json");
        }));

    ConnectedRealityDataDocument doc = ConnectedRealityDataDocument("72adad30-c07c-465d-a1fe-2f2dfac950a7");

    EXPECT_EQ(doc.GetName(), "Production_Helsinki_3MX_ok.3mx");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataFolderTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Folder/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a7");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\RealityDataFolder.json");
        }));

    ConnectedRealityDataFolder folder = ConnectedRealityDataFolder("72adad30-c07c-465d-a1fe-2f2dfac950a7");

    EXPECT_EQ(folder.GetName(), "Scene123");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a5");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\SingleRealityData-Helsinki.json");
        }));

    ConnectedRealityData rd = ConnectedRealityData("72adad30-c07c-465d-a1fe-2f2dfac950a5");
    
    EXPECT_EQ(rd.GetName(), "Helsinki");
    }