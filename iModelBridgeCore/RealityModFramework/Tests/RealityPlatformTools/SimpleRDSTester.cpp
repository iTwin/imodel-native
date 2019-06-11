//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include "../../RealityPlatformTools/RealityDataServiceInternal.h"
#include "../Common/RealityModFrameworkTestsCommon.h"
#include <RealityPlatformTools/SimpleRDSApi.h>

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

//=====================================================================================
//! @bsiclass                                   Spencer.Mason                  10/2017
//! SimpleRDSFixture
//=====================================================================================
class SimpleRDSFixture : public MockWSGRequestFixture
    {
public:
    static RealityDataService* s_realityDataServices;

    static void SetUpTestCase()
        {
        s_mockWSGInstance = new MockWSGRequest();
        s_realityDataServices = new RealityDataService();
        s_realityDataServices->SetServerComponents("myserver.com", "9.9", "myRepo", "mySchema", "zz:\\mycertificate.pfx", "myProjectID");
        s_realityDataServices->SetUserAgent("My User Agent 1.0");
        }

    static void TearDownTestCase()
        {
        delete s_realityDataServices;
        s_realityDataServices = nullptr;
        delete s_mockWSGInstance;
        s_mockWSGInstance = nullptr;
        }
    };

struct MockRDSRequestManager : RDSRequestManager
    {   
    MockRDSRequestManager() : RDSRequestManager()
        {}

    ~MockRDSRequestManager()
        {
        }

    MOCK_METHOD0(Setup, void());
    };

RealityDataService* SimpleRDSFixture::s_realityDataServices = nullptr;

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  12/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, BuddiTest)
    {
    Utf8String url = RDSRequestManager::MakeBuddiCall(L"RealityDataServices");

    EXPECT_TRUE(url.ContainsI("realitydataservices"));
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  01/2018
//=====================================================================================
/*TEST_F(SimpleRDSFixture, RDSRequestManagerTest)
    {
    RDSRequestManager::Setup("");

    ASSERT_TRUE(!RealityDataService::GetServerName().empty());
    ASSERT_TRUE(!RealityDataService::GetWSGProtocol().empty());
    ASSERT_EQ(RealityDataService::GetRepoName(), "S3MXECPlugin--Server");
    ASSERT_EQ(RealityDataService::GetSchemaName(), "S3MX");
    ASSERT_EQ(RealityDataService::GetUserAgent(), "My User Agent");
    }*/

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  01/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedNavNodeTest)
    {
    Utf8String schema = "schema"; 
    Utf8String id = "id"; 
    Utf8String typeSystem = "typeSystem"; 
    Utf8String rootId = "rootId";

    NavNode originNode = NavNode(schema, id, typeSystem, rootId);

    ConnectedNavNode cNode = ConnectedNavNode(originNode);

    EXPECT_STREQ(originNode.GetSchemaName().c_str(), cNode.GetSchemaName().c_str());
    EXPECT_STREQ(originNode.GetInstanceId().c_str(), cNode.GetInstanceId().c_str());
    EXPECT_STREQ(originNode.GetTypeSystem().c_str(), cNode.GetTypeSystem().c_str());
    EXPECT_STREQ(originNode.GetRootId().c_str(), cNode.GetRootId().c_str());
    }


//=====================================================================================
//! @bsimethod                                  Alain.Robert                  05/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataLocationTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\DataLocation.json");
        }));

    ConnectedRealityDataLocation location("54494b32-149a-4177-a849-189877553854");
    
    EXPECT_EQ(location.GetProvider(), "Microsoft");
    }

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
    ConnectedResponse response = stat.GetEnterpriseStats();
    
    EXPECT_TRUE(response.simpleSuccess);
    EXPECT_EQ(stat.GetUltimateId(), "e82a584b-9fae-409f-9581-fd154f7b9ef9");
    }

//=====================================================================================
//! @bsimethod                                  Alain.Robert                  04/2018
//=====================================================================================
// TEST_F(SimpleRDSFixture, ConnectedRealityDataServiceStatTest)
    // {
    // EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        // {
        // response.status = ::OK;
        // response.responseCode = 200;
        // response.toolCode = CURLE_OK;
        // response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\ServiceStat.json");
        // }));

    // ConnectedRealityDataServiceStat stat = ConnectedRealityDataServiceStat();
    // ConnectedResponse response = stat.GetServiceStats();
    
    // EXPECT_TRUE(response.simpleSuccess);
    // EXPECT_EQ(stat.GetUltimateId(), "53dd5a3b-929e-4169-b2e7-afce74a1d0af");
    // }

//=====================================================================================
//! @bsimethod                                  Alain.Robert                  04/2018
//=====================================================================================
// TEST_F(SimpleRDSFixture, ConnectedRealityDataUserStatTest)
    // {
    // EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        // {
        // response.status = ::OK;
        // response.responseCode = 200;
        // response.toolCode = CURLE_OK;
        // response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\UserStat.json");
        // }));

    // ConnectedRealityDataUserStat stat = ConnectedRealityDataUserStat();
    // ConnectedResponse response = stat.GetUserStats();
    
    // EXPECT_TRUE(response.simpleSuccess);
    // EXPECT_EQ(stat.GetUltimateId(), "72adad30-c07c-465d-a1fe-2f2dfac950a4");
    // }
	
//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
/*TEST_F(SimpleRDSFixture, ConnectedRealityDataEnterpriseStatsTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\EnterpriseStat.json");
        }));

    bvector<ConnectedRealityDataEnterpriseStat> stats = bvector<ConnectedRealityDataEnterpriseStat>();
    ConnectedResponse response = ConnectedRealityDataEnterpriseStat::GetAllStats(stats);

    EXPECT_TRUE(response.simpleSuccess);
    EXPECT_EQ(stats[0].GetUltimateId(), "e82a584b-9fae-409f-9581-fd154f7b9ef9");
    }*/
//=====================================================================================
//! @bsimethod                                  Alain.Robert                 05/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataLocationCloneTest)
    {
    RealityDataLocation location;

    location.SetIdentifier("Dad");
    location.SetProvider("MyProvider");
    location.SetLocation("Mordor");

    ConnectedRealityDataLocation cLocation(location);

    EXPECT_EQ(location.GetIdentifier(), cLocation.GetIdentifier());
    EXPECT_EQ(location.GetProvider(), cLocation.GetProvider());
    EXPECT_EQ(location.GetLocation(), cLocation.GetLocation());
    }
//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataEnterpriseStatCloneTest)
    {
    RealityDataEnterpriseStat stat;

    stat.SetNbRealityData(100);
    stat.SetTotalSizeKB(1000.0);
    stat.SetOrganizationId("orgID");
    stat.SetUltimateId("ultID");
    stat.SetUltimateSite("ultSite");
    stat.SetDate(DateTime::GetCurrentTimeUtc());

    ConnectedRealityDataEnterpriseStat cStat = ConnectedRealityDataEnterpriseStat(stat);

    EXPECT_EQ(stat.GetNbRealityData(), cStat.GetNbRealityData());
    EXPECT_EQ(stat.GetTotalSizeKB(), cStat.GetTotalSizeKB());
    EXPECT_EQ(stat.GetOrganizationId(), cStat.GetOrganizationId());
    EXPECT_EQ(stat.GetUltimateId(), cStat.GetUltimateId());
    EXPECT_EQ(stat.GetUltimateSite(), cStat.GetUltimateSite());
    EXPECT_EQ(stat.GetDate(), cStat.GetDate());
    }

//=====================================================================================
//! @bsimethod                                  Alain.Robert                  04/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataUserStatCloneTest)
    {
    RealityDataUserStat stat;

    stat.SetNbRealityData(100);
    stat.SetTotalSizeKB(1000.0);
    stat.SetUserId("usrID");
    stat.SetUserEmail("usrEmail");
    stat.SetUltimateId("ultID");
	stat.SetServiceId("4686");
    stat.SetDataLocationGuid("dataGuid");
    stat.SetDate(DateTime::GetCurrentTimeUtc());

    ConnectedRealityDataUserStat cStat(stat);

    EXPECT_EQ(stat.GetNbRealityData(), cStat.GetNbRealityData());
    EXPECT_EQ(stat.GetTotalSizeKB(), cStat.GetTotalSizeKB());
    EXPECT_EQ(stat.GetUserId(), cStat.GetUserId());
    EXPECT_EQ(stat.GetUserEmail(), cStat.GetUserEmail());
    EXPECT_EQ(stat.GetUltimateId(), cStat.GetUltimateId());
    EXPECT_EQ(stat.GetServiceId(), cStat.GetServiceId());
    EXPECT_EQ(stat.GetDataLocationGuid(), cStat.GetDataLocationGuid());
    EXPECT_EQ(stat.GetDate(), cStat.GetDate());
    }
//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataRelationshipIdRequestTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship?$filter=RealityDataId+eq+'72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a6'");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\RealityDataRelationship.json");
        }));

    ConnectedRealityDataRelationship rel = ConnectedRealityDataRelationship();
    rel.SetRealityDataId("72adad30-c07c-465d-a1fe-2f2dfac950a6");
    bvector<ConnectedRealityDataRelationshipPtr> results;
    ConnectedResponse response = rel.RetrieveAllForRDId(results);

    EXPECT_TRUE(response.simpleSuccess);
    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0]->GetRealityDataId(), "f4425509-55c4-4e03-932a-d67b87ace30f");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataRelationshipIdBadRequestTest)
    {
    ConnectedRealityDataRelationship rel = ConnectedRealityDataRelationship();
    bvector<ConnectedRealityDataRelationshipPtr> results;
    ConnectedResponse response = rel.RetrieveAllForRDId(results);

    EXPECT_FALSE(response.simpleSuccess);
    EXPECT_EQ(response.simpleMessage, "must set realityData id, first");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataRelationshipProjectIdRequestTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship?$filter=RelatedId+eq+'72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a6'");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\RealityDataRelationship.json");
        }));

    ConnectedRealityDataRelationship rel = ConnectedRealityDataRelationship();
    rel.SetRelatedId("72adad30-c07c-465d-a1fe-2f2dfac950a6");
    bvector<ConnectedRealityDataRelationshipPtr> results;
    ConnectedResponse response = rel.RetrieveAllForProjectId(results);

    EXPECT_TRUE(response.simpleSuccess);
    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0]->GetRealityDataId(), "f4425509-55c4-4e03-932a-d67b87ace30f");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataRelationshipProjectIdBadRequestTest)
    {
    ConnectedRealityDataRelationship rel = ConnectedRealityDataRelationship();
    bvector<ConnectedRealityDataRelationshipPtr> results;
    ConnectedResponse response = rel.RetrieveAllForProjectId(results);

    EXPECT_FALSE(response.simpleSuccess);
    EXPECT_EQ(response.simpleMessage, "must set related id, first");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataRelationshipDeleteRequestTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityDataRelationship/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a6~2F72adad30-c07c-465d-a1fe-2f2dfac950a6");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\Deleted.json");
        }));

    ConnectedRealityDataRelationship rel = ConnectedRealityDataRelationship();
    rel.SetRealityDataId("72adad30-c07c-465d-a1fe-2f2dfac950a6");
    rel.SetRelatedId("72adad30-c07c-465d-a1fe-2f2dfac950a6");
    ConnectedResponse response = rel.Delete();

    EXPECT_TRUE(response.simpleSuccess);
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataRelationshipDeleteBadRequestTest)
    {
    ConnectedRealityDataRelationship rel = ConnectedRealityDataRelationship();
    ConnectedResponse response = rel.Delete();

    EXPECT_FALSE(response.simpleSuccess);
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataRelationshipCreateRequestTest)
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

    ConnectedRealityDataRelationship rel = ConnectedRealityDataRelationship();
    rel.SetRealityDataId("MyIdentifier");
    ConnectedResponse response = rel.CreateOnServer();

    EXPECT_FALSE(response.GetSuccess());

    rel.SetRelatedId("MYProjectID");
    response = rel.CreateOnServer();

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
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataDocumentDeleteRequestTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(2).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Document/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a7");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;

        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\RealityDataDocument.json");
        })).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Document/43a4a51a%2Dbfd3%2D4271%2Da9d9%2D21db56cdcf10%7E2FScene%7E2FProduction%5FHelsinki%5F3MX%5Fok%2E3mx");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\Deleted.json");
        }));

    ConnectedRealityDataDocument doc = ConnectedRealityDataDocument("72adad30-c07c-465d-a1fe-2f2dfac950a7");
    ConnectedResponse response = doc.Delete();

    EXPECT_TRUE(response.simpleSuccess);
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataDocumentDeleteBadRequestTest)
    {
    ConnectedRealityDataDocument doc = ConnectedRealityDataDocument();
    ConnectedResponse response = doc.Delete();

    EXPECT_FALSE(response.simpleSuccess);
    EXPECT_EQ(response.simpleMessage, "must set server path to document (id), first");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataDocumentGetInfoBadRequestTest)
    {
    ConnectedRealityDataDocument doc = ConnectedRealityDataDocument();
    ConnectedResponse response = doc.GetInfo();

    EXPECT_FALSE(response.simpleSuccess);
    EXPECT_EQ(response.simpleMessage, "must set server path to document (id), first");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataAllDocumentsTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a7/FileAccess.FileAccessKey?$filter=Permissions+eq+'Read'&api.singleurlperinstance=true");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;

        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\AzureHandshake.json");
        }));
        
    EXPECT_CALL(*s_mockWSGInstance, PerformAzureRequest(_, _, _, _, _)).Times(2).WillRepeatedly(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        auto requestString = wsgRequest.GetHttpRequestString();
        if (requestString.Contains("marker=Page2"))
            {
            response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\ListAllServerResponseSecondPage.xml");
            }
        else
            {
            response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\ListAllServerResponseFirstPage.xml");
            }

        }));

    bvector<bpair<Utf8String, uint64_t>> docVector;

    ConnectedResponse response = ConnectedRealityDataDocument::RetrieveAllForRealityData(docVector, "72adad30-c07c-465d-a1fe-2f2dfac950a7");

    EXPECT_EQ(docVector[0].first, "72adad30-c07c-465d-a1fe-2f2dfac950a7/Folder1/File1.txt");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataAllDocumentsBadTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/RealityData/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a7/FileAccess.FileAccessKey?$filter=Permissions+eq+'Read'&api.singleurlperinstance=true");
        response.status = ::BADREQ;
        response.responseCode = 404;
        response.toolCode = 55;
        }));

    bvector<bpair<Utf8String, uint64_t>> docVector;

    ConnectedResponse response = ConnectedRealityDataDocument::RetrieveAllForRealityData(docVector, "72adad30-c07c-465d-a1fe-2f2dfac950a7");

    EXPECT_FALSE(response.simpleSuccess);
    EXPECT_EQ(response.simpleMessage, "Failure retrieving Azure token\n");
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
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataFolderDeleteRequestTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(2).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Folder/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a7");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\Deleted.json");
        })).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        EXPECT_STREQ(wsgRequest.GetHttpRequestString().c_str(), "https://myserver.com/v9.9/Repositories/myRepo/mySchema/Folder/72adad30%2Dc07c%2D465d%2Da1fe%2D2f2dfac950a7");
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\Deleted.json");
        }));

    ConnectedRealityDataFolder folder = ConnectedRealityDataFolder("72adad30-c07c-465d-a1fe-2f2dfac950a7");
    ConnectedResponse response = folder.Delete();

    EXPECT_TRUE(response.simpleSuccess);
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataFolderDeleteBadRequestTest)
    {
    ConnectedRealityDataFolder folder = ConnectedRealityDataFolder();
    ConnectedResponse response = folder.Delete();

    EXPECT_FALSE(response.simpleSuccess);
    EXPECT_EQ(response.simpleMessage, "must set server path to document (id), first");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  02/2018
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataFolderGetInfoBadRequestTest)
    {
    ConnectedRealityDataFolder folder = ConnectedRealityDataFolder();
    ConnectedResponse response = folder.Delete();

    EXPECT_FALSE(response.simpleSuccess);
    EXPECT_EQ(response.simpleMessage, "must set server path to document (id), first");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataTest)
    {
    ConnectedRealityData emptyRd = ConnectedRealityData();

    EXPECT_TRUE(emptyRd.GetMetadataUrl().empty());

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

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataGetInfoBadTest)
    {
    ConnectedRealityData emptyRd = ConnectedRealityData();
    ConnectedResponse response = emptyRd.GetInfo();

    EXPECT_FALSE(response.simpleSuccess);
    EXPECT_EQ(response.simpleMessage, "must set ultimate id, first");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataUpdateBadTest)
    {
    ConnectedRealityData emptyRd = ConnectedRealityData();
    ConnectedResponse response = emptyRd.UpdateInfo();

    EXPECT_FALSE(response.simpleSuccess);
    EXPECT_EQ(response.simpleMessage, "must set ultimate id, first");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataDeleteBadTest)
    {
    ConnectedRealityData emptyRd = ConnectedRealityData();
    ConnectedResponse response = emptyRd.Delete();

    EXPECT_FALSE(response.simpleSuccess);
    EXPECT_EQ(response.simpleMessage, "must set server path to document (id), first");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataForAllUltTest)
    {
    EXPECT_CALL(*s_mockWSGInstance, PerformRequest(_, _, _, _, _)).Times(1).WillOnce(Invoke([](const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry)
        {
        response.status = ::OK;
        response.responseCode = 200;
        response.toolCode = CURLE_OK;
        response.body = RealityModFrameworkTestsUtils::GetTestDataContent(L"TestData\\RealityPlatformTools\\MultipleRealityData.json");
        }));

    ConnectedRealityData emptyRd = ConnectedRealityData();
    emptyRd.SetUltimateId("72adad30-c07c-465d-a1fe-2f2dfac950a5");

    bvector<ConnectedRealityDataPtr> dataVector;
    ConnectedResponse response = emptyRd.RetrieveAllForUltimateId(dataVector);

    EXPECT_EQ(dataVector[0]->GetName(), "Helsinki");
    }

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleRDSFixture, ConnectedRealityDataForAllUltBadTest)
    {
    bvector<ConnectedRealityDataPtr> dataVector;
    ConnectedRealityData emptyRd = ConnectedRealityData();
    ConnectedResponse response = emptyRd.RetrieveAllForUltimateId(dataVector);

    EXPECT_FALSE(response.simpleSuccess);
    EXPECT_EQ(response.simpleMessage, "must set ultimate id, first");
    }