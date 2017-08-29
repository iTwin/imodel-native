/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/Retry_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include "IntegrationTestsHelper.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTest.h>
#include "../BackDoor/PublicAPI/BackDoor/WebServices/iModelHub/BackDoor.h"
#include "MockHttpHandler.h"
#include "../../../Client/ServerInfoProvider.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

//---------------------------------------------------------------------------------------
//@bsiclass                                     Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
struct RetryTests : public IntegrationTestsBase
    {
    ClientPtr    m_client;
    iModelInfoPtr m_imodel;
    iModelConnectionPtr m_imodelConnection;
    const wchar_t*  m_briefcasePathFileName = L"BreakTestPath.txt";
    
    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();
        ServerInfoProvider::InvalidateAllInfo();

        auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
        m_client = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidAdminCredentials(), proxy);
        m_imodel = CreateNewiModel(*m_client, nullptr);
        m_imodelConnection = ConnectToiModel(*m_client, m_imodel);

        m_pHost->SetRepositoryAdmin(m_client->GetiModelAdmin());
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    virtual void TearDown() override
        {
        DeleteiModel(*m_client, *m_imodel);
        m_client = nullptr;
        IntegrationTestsBase::TearDown();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    IRepositoryManagerP _GetRepositoryManager(DgnDbR db) const
        {
        return m_client->GetiModelAdmin()->_GetRepositoryManager(db);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Algirdas.Mikoliunas             10/2016
    //---------------------------------------------------------------------------------------
    BriefcasePtr AcquireBriefcase()
        {
        return IntegrationTestsBase::AcquireBriefcase(*m_client, *m_imodel);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Algirdas.Mikoliunas             09/2016
    //---------------------------------------------------------------------------------------
    void CreateAndPushModel(BriefcasePtr briefcase)
        {
        auto model2 = CreateModel("Model2", briefcase->GetDgnDb());
        briefcase->GetDgnDb().SaveChanges();
        auto rev2 = PushPendingChanges(*briefcase);
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
Response StubHttpResponse(HttpStatus httpStatus, HttpBodyPtr body, const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String>())
    {
    ConnectionStatus status = ConnectionStatus::OK;
    if (httpStatus == HttpStatus::None)
        {
        status = ConnectionStatus::CouldNotConnect;
        }
    auto content = HttpResponseContent::Create(body);
    for (const auto& header : headers)
        {
        content->GetHeaders().SetValue(header.first, header.second);
        }
    return Http::Response(content, "", status, httpStatus);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
Response StubHttpResponse(HttpStatus httpStatus, Utf8StringCR body = "", const std::map<Utf8String, Utf8String>& headers = std::map<Utf8String, Utf8String>())
    {
    return StubHttpResponse(httpStatus, HttpStringBody::Create(body), headers);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
IWSRepositoryClientPtr CreateWSClient(iModelInfoPtr imodel, std::shared_ptr<MockHttpHandler> mockHandler)
    {
    WebServices::ClientInfoPtr clientInfo = IntegrationTestSettings::Instance().GetClientInfo();
    return WSRepositoryClient::Create(imodel->GetServerURL(), imodel->GetWSRepositoryName(), clientInfo, nullptr, mockHandler);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
TEST_F(RetryTests, QueryChangeSetsFails)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    ExpectLocksCount(*briefcase1, 0);

    //Create model in briefcase. This should also acquire locks automatically.
    auto partition = CreateAndInsertModeledElement ("Model1", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();

    auto imodelConnection = BackDoor::Briefcase::GetiModelConnectionPtr(briefcase1);
    IWSRepositoryClientPtr oldWSClient = BackDoor::iModelConnection::GetRepositoryClient(imodelConnection);
    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(4)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::OK, "", { {"Server" , "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00"}});
            }
        ).ForRequest(2, [=](Http::RequestCR request)
            {
            // Query changeSet instances fails
            return StubHttpResponse(HttpStatus::ReqestTimeout, "", { { "Content-Type" , "application/json" } });
            }
        ).ForRequest(3, [=](Http::RequestCR request)
                {
                return StubHttpResponse(HttpStatus::OK, "", { { "Server" , "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00" } });
                }
        ).ForRequest(4, [=](Http::RequestCR request)
            {
            // Query changeSet instances fails
            BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, oldWSClient);
            return StubHttpResponse(HttpStatus::ReqestTimeout, "", { { "Content-Type" , "application/json" } });
            }
        );
    
    // Set other wsclient
    auto newClient = CreateWSClient(m_imodel, mockHandler);
    BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, newClient);
    
    // First push should fail
    auto pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_FALSE(pushResult.IsSuccess());
    
    // Second push should succeed
    pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    auto imodelManager = _GetRepositoryManager(briefcase1->GetDgnDb());
    ExpectCodeState(CreateCodeUsed(partition->GetCode(), briefcase1->GetLastChangeSetPulled()), imodelManager);
    CreateAndPushModel(briefcase1);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(RetryTests, QueryChangeSetsFailsSingleTime)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    ExpectLocksCount(*briefcase1, 0);

    //Create model in briefcase. This should also acquire locks automatically.
    auto partition = CreateAndInsertModeledElement ("Model1", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();

    auto imodelConnection = BackDoor::Briefcase::GetiModelConnectionPtr(briefcase1);
    IWSRepositoryClientPtr oldWSClient = BackDoor::iModelConnection::GetRepositoryClient(imodelConnection);
    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(2)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::OK, "", { {"Server" , "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00"}});
            }
        ).ForRequest(2, [=](Http::RequestCR request)
            {
            // Query changeSet instances fails
            BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, oldWSClient);
            return StubHttpResponse(HttpStatus::ReqestTimeout, "", { { "Content-Type" , "application/json" } });
            }
        );
    
    // Set other wsclient
    auto newClient = CreateWSClient(m_imodel, mockHandler);
    BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, newClient);
    
    // First push should pass
    auto pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    auto imodelManager = _GetRepositoryManager(briefcase1->GetDgnDb());
    ExpectCodeState(CreateCodeUsed(partition->GetCode(), briefcase1->GetLastChangeSetPulled()), imodelManager);
    CreateAndPushModel(briefcase1);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
TEST_F(RetryTests, CreateChangeSetFails)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    ExpectLocksCount(*briefcase1, 0);

    //Create model in briefcase. This should also acquire locks automatically.
    auto partition = CreateAndInsertModeledElement ("Model1", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();

    auto imodelConnection = BackDoor::Briefcase::GetiModelConnectionPtr(briefcase1);
    IWSRepositoryClientPtr oldWSClient = BackDoor::iModelConnection::GetRepositoryClient(imodelConnection);

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(4)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            // Query changeSet instances
            return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
            }
        ).ForRequest(2, [=](Http::RequestCR request)
            {
            // Create changeSet in the server fails
            return StubHttpResponse(HttpStatus::InternalServerError);
            }
        ).ForRequest(3, [=](Http::RequestCR request)
            {
            // Query changeSet instances
            return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
            }
        ).ForRequest(4, [=](Http::RequestCR request)
            {
            // Create changeSet in the server fails
            BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, oldWSClient);
            return StubHttpResponse(HttpStatus::InternalServerError);
            }
        );
    
    // Set other wsclient
    auto newClient = CreateWSClient(m_imodel, mockHandler);
    BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, newClient);

    // First push should fail
    auto pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_FALSE(pushResult.IsSuccess());

    // Second push should succeed
    pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);

    auto imodelManager = _GetRepositoryManager(briefcase1->GetDgnDb());
    ExpectCodeState(CreateCodeUsed(partition->GetCode(), briefcase1->GetLastChangeSetPulled()), imodelManager);
    CreateAndPushModel(briefcase1);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
TEST_F(RetryTests, UploadFileFails)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    ExpectLocksCount(*briefcase1, 0);

    //Create model in briefcase. This should also acquire locks automatically.
    auto partition = CreateAndInsertModeledElement ("Model1", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();

    auto imodelConnection = BackDoor::Briefcase::GetiModelConnectionPtr(briefcase1);
    IWSRepositoryClientPtr oldWSClient = BackDoor::iModelConnection::GetRepositoryClient(imodelConnection);

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(6)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::OK, "", { { "Server" , "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00" } });
            }
        ).ForRequest(2, [=](Http::RequestCR request)
            {
            // Query changeSet instances
            return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
            }
        ).ForRequest(3, [=](Http::RequestCR request)
            {
            // Create changeSet in server
            return StubHttpResponse(HttpStatus::Created);
            }
        ).ForRequest(4, [=](Http::RequestCR request)
            {
            // Upload file
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        ).ForRequest(5, [=](Http::RequestCR request)
            {
            // Create changeSet in server
            return StubHttpResponse(HttpStatus::Created);
            }
        ).ForRequest(6, [=](Http::RequestCR request)
            {
            // Upload file
            BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, oldWSClient);
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        );

    // Set other wsclient
    auto newClient = CreateWSClient(m_imodel, mockHandler);
    BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, newClient);

    // First push should fail
    auto pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_FALSE(pushResult.IsSuccess());

    // Second push should succeed
    pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);

    auto imodelManager = _GetRepositoryManager(briefcase1->GetDgnDb());
    ExpectCodeState(CreateCodeUsed(partition->GetCode(), briefcase1->GetLastChangeSetPulled()), imodelManager);
    CreateAndPushModel(briefcase1);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
TEST_F(RetryTests, InitializeiModelFails)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    ExpectLocksCount(*briefcase1, 0);

    //Create model in briefcase. This should also acquire locks automatically.
    auto partition = CreateAndInsertModeledElement ("Model1", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();

    auto imodelConnection = BackDoor::Briefcase::GetiModelConnectionPtr(briefcase1);
    IWSRepositoryClientPtr oldWSClient = BackDoor::iModelConnection::GetRepositoryClient(imodelConnection);

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(8)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            // Query changeSet instances
            return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
            }
        ).ForRequest(2, [=](Http::RequestCR request)
            {
            // Create changeSet in server
            return StubHttpResponse(HttpStatus::Created);
            }
        ).ForRequest(3, [=](Http::RequestCR request)
            {
            // Upload file
            return StubHttpResponse(HttpStatus::OK);
            }
        ).ForRequest(4, [=](Http::RequestCR request)
            {
            // Confirm and send locks/codes
            return StubHttpResponse(HttpStatus::BadRequest);
            }
        ).ForRequest(5, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::OK, "", { { "Server" , "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00" } });
            }
        ).ForRequest(6, [=](Http::RequestCR request)
            {
            // Create changeSet in server
            return StubHttpResponse(HttpStatus::Created);
            }
        ).ForRequest(7, [=](Http::RequestCR request)
            {
            // Upload file
            return StubHttpResponse(HttpStatus::OK);
            }
        ).ForRequest(8, [=](Http::RequestCR request)
            {
            // Confirm and send locks/codes
            BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, oldWSClient);
            return StubHttpResponse(HttpStatus::BadRequest);
            }
        );

    // Set other wsclient
    auto newClient = CreateWSClient(m_imodel, mockHandler);
    BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, newClient);
    
    // First push should fail
    auto pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_FALSE(pushResult.IsSuccess());

    // Second push should succeed
    pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);

    auto imodelManager = _GetRepositoryManager(briefcase1->GetDgnDb());
    ExpectCodeState(CreateCodeUsed(partition->GetCode(), briefcase1->GetLastChangeSetPulled()), imodelManager);
    CreateAndPushModel(briefcase1);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
TEST_F(RetryTests, DownloadedChangeSetInvalid)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    ExpectLocksCount(*briefcase1, 0);

    //Create model in briefcase 1. This should also acquire locks automatically.
    auto partition = CreateAndInsertModeledElement ("Model1", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();

    auto imodelConnection = BackDoor::Briefcase::GetiModelConnectionPtr(briefcase1);
    IWSRepositoryClientPtr oldWSClient = BackDoor::iModelConnection::GetRepositoryClient(imodelConnection);

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(2)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            // Query changeSet instances
            return StubHttpResponse(HttpStatus::OK, "{\"instances\":"
                "[{\"instanceId\":\"00000000-0000-0000-0000-0000000000000000\","
                "\"schemaName\":\"iModelScope\","
                "\"className\":\"ChangeSet\","
                "\"properties\":{"
                    "\"FileName\":\"00000000-0000-0000-0000-000000000000.rev\","
                    "\"Description\":\"\","
                    "\"URL\":null,"
                    "\"FileSize\":\"0\","
                    "\"Index\":1,"
                    "\"Id\":\"00000000-0000-0000-0000-0000000000000000\","
                    "\"ParentId\":\"\","
                    "\"MasterFileId\":\"GUID-0\","
                    "\"BriefcaseId\":2,"
                    "\"UserCreated\":\"test\","
                    "\"PushDate\":\"2016-09-28T05:43:55.2\","
                    "\"ContainingChanges\":0,"
                    "\"IsUploaded\":true"
                "}}]}");
            }
        ).ForRequest(2, [=](Http::RequestCR request)
            {
            // Download changeSet
            BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, oldWSClient);
            return StubHttpResponse(HttpStatus::OK, "Test");
            }
        );

    // Set other wsclient
    auto newClient = CreateWSClient(m_imodel, mockHandler);
    BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, newClient);

    // First push should fail
    BeTest::SetFailOnAssert(false);
    auto pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
    BeTest::SetFailOnAssert(true);
    
    EXPECT_FALSE(pushResult.IsSuccess());
    EXPECT_EQ(Error::Id::RevisionManagerError, pushResult.GetError().GetId());

    // Second push should succeed
    pushResult = briefcase1->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);

    auto imodelManager = _GetRepositoryManager(briefcase1->GetDgnDb());
    ExpectCodeState(CreateCodeUsed(partition->GetCode(), briefcase1->GetLastChangeSetPulled()), imodelManager);
    CreateAndPushModel(briefcase1);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(RetryTests, AcquireCodesLocksFails)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    ExpectLocksCount(*briefcase1, 0);

    iModelConnectionPtr imodelConnection;
    IWSRepositoryClientPtr oldWSClient;

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(6)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            // Query held resources
            return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
            }
        ).ForRequest(2, [=](Http::RequestCR request)
            {
            // Query unavailable locks/codes
            return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
            }
        ).ForRequest(3, [=](Http::RequestCR request)
            {
            // Query unavailable locks/codes
            return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
            }
        ).ForRequest(4, [=](Http::RequestCR request)
            {
            // Query unavailable locks/codes
            return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
            }
        ).ForRequest(5, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        ).ForRequest(6, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        );

    DgnDbR db = briefcase1->GetDgnDb();
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), "PartitionModel1");
    EXPECT_TRUE(partition.IsValid());

    iModelManager* manager = (iModelManager*)_GetRepositoryManager(db);
    imodelConnection = BackDoor::iModelManager::GetiModelConnectionPtr(manager);
    oldWSClient = BackDoor::iModelConnection::GetRepositoryClient(imodelConnection);

    // Set other wsclient
    auto newClient = CreateWSClient(m_imodel, mockHandler);
    BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, newClient);

    auto stat = db.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::ServerUnavailable, stat);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(RetryTests, AcquireCodesLocksSingleFailSucceeds)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    ExpectLocksCount(*briefcase1, 0);

    iModelConnectionPtr imodelConnection;
    IWSRepositoryClientPtr oldWSClient;

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(5)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::OK, "", { { "Server" , "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00" } });
            }
        ).ForRequest(2, [=](Http::RequestCR request)
            {
            // Query held resources
            return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
            }
        ).ForRequest(3, [=](Http::RequestCR request)
            {
            // Query unavailable locks/codes
            return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
            }
        ).ForRequest(4, [=](Http::RequestCR request)
                {
                // Query unavailable locks/codes
                return StubHttpResponse(HttpStatus::OK, "{instances: {}}");
                }
        ).ForRequest(5, [&](Http::RequestCR request)
            {
            BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, oldWSClient);
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        );
    
    DgnDbR db = briefcase1->GetDgnDb();
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), "PartitionModel1");
    EXPECT_TRUE(partition.IsValid());

    iModelManager* manager = (iModelManager*)_GetRepositoryManager(db);
    imodelConnection = BackDoor::iModelManager::GetiModelConnectionPtr(manager);
    oldWSClient = BackDoor::iModelConnection::GetRepositoryClient(imodelConnection);

    // Set other wsclient
    auto newClient = CreateWSClient(m_imodel, mockHandler);
    BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, newClient);

    auto stat = db.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::Success, stat);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(RetryTests, AcquireCodesLocksQueryHeldResourcesFails)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();

    iModelConnectionPtr imodelConnection;
    IWSRepositoryClientPtr oldWSClient;

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(8)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            // iModelScope/MultiCode?$filter=(BriefcaseId+eq+2)
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        ).ForRequest(2, [&](Http::RequestCR request)
            {
            // iModelScope/MultiCode?$filter=(BriefcaseId+eq+2)
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        ).ForRequest(3, [&](Http::RequestCR request)
            {
            // iModelScope/MultiLock?$filter=(BriefcaseId+eq+2)
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        ).ForRequest(4, [&](Http::RequestCR request)
            {
            // iModelScope/MultiLock?$filter=(BriefcaseId+eq+2)
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        ).ForRequest(5, [=](Http::RequestCR request)
            {
            // iModelScope/Code?$filter=BriefcaseId+ne+2
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        ).ForRequest(6, [&](Http::RequestCR request)
            {
            // iModelScope/Code?$filter=BriefcaseId+ne+2
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        ).ForRequest(7, [&](Http::RequestCR request)
            {
            // iModelScope/Lock?$filter=BriefcaseId+ne+2+and+(LockLevel+gt+0+or+ReleasedWithChangeSetIndex+gt+0)
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        ).ForRequest(8, [&](Http::RequestCR request)
            {
            BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, oldWSClient);
            // iModelScope/Lock?$filter=BriefcaseId+ne+2+and+(LockLevel+gt+0+or+ReleasedWithChangeSetIndex+gt+0)
            return StubHttpResponse(HttpStatus::ReqestTimeout,
                "{\"errorId\": \"\", "
                "\"errorMessage\" : \"Request timed out\", "
                "\"errorDescription\" : \"Request timed out\"}", { { "Content-Type" , "application/json" } });
            }
        );
    
    DgnDbR db = briefcase1->GetDgnDb();
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), "PartitionModel1");
    EXPECT_TRUE(partition.IsValid());

    iModelManager* manager = (iModelManager*)_GetRepositoryManager(db);
    imodelConnection = BackDoor::iModelManager::GetiModelConnectionPtr(manager);
    oldWSClient = BackDoor::iModelConnection::GetRepositoryClient(imodelConnection);

    // Set other wsclient
    auto newClient = CreateWSClient(m_imodel, mockHandler);
    BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, newClient);

    auto stat = db.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::ServerUnavailable, stat);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(RetryTests, DownloadSeedFileRetries)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();

    auto imodelConnection = BackDoor::Briefcase::GetiModelConnectionPtr(briefcase1);
    IWSRepositoryClientPtr oldWSClient = BackDoor::iModelConnection::GetRepositoryClient(imodelConnection);

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(3)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            // Download seed file
            return StubHttpResponse(HttpStatus::ReqestTimeout);
            }
        ).ForRequest(2, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::OK, "", { { "Server" , "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00" } });
            }
        ).ForRequest(3, [=](Http::RequestCR request)
            {
            BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, oldWSClient);
            // Download seed file retry
            return StubHttpResponse(HttpStatus::ReqestTimeout);
            }
        );
    
    // Set other wsclient
    auto newClient = CreateWSClient(m_imodel, mockHandler);
    BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, newClient);

    auto path = m_pHost->GetOutputDirectory();
    FileInfoPtr fileInfo = FileInfo::Create(briefcase1->GetDgnDb(), "Replacement description0");
    auto stat = briefcase1->GetiModelConnection().DownloadSeedFile(path, fileInfo->GetFileId().ToString())->GetResult();
    EXPECT_FALSE (stat.IsSuccess());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(RetryTests, DownloadSeedFileSingleFailSucceeds)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();

    auto imodelConnection = BackDoor::Briefcase::GetiModelConnectionPtr(briefcase1);
    IWSRepositoryClientPtr oldWSClient = BackDoor::iModelConnection::GetRepositoryClient(imodelConnection);

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(4)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::OK, "{\"instances\":[{\"schemaName\": \"iModelScope\", \"className\": \"SeedFile\", \"instanceId\" : \"Id\", \"properties\":{}}]}");
            }
        ).ForRequest(2, [=](Http::RequestCR request)
            {
            // Download seed file
            return StubHttpResponse(HttpStatus::ReqestTimeout);
            })
        .ForRequest(3, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::OK, "", { { "Server" , "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00" } });
            }
        ).ForRequest(4, [=](Http::RequestCR request)
            {
            // Second download succeeds
            return StubHttpResponse(HttpStatus::OK, "Content");
            }
        );
    
    // Set other wsclient
    auto newClient = CreateWSClient(m_imodel, mockHandler);
    BackDoor::iModelConnection::SetRepositoryClient(imodelConnection, newClient);

    auto path = m_pHost->GetOutputDirectory().AppendToPath(L"SeedFileRetry.bim");
    FileInfoPtr fileInfo = FileInfo::Create(briefcase1->GetDgnDb(), "Replacement description0");
    auto stat = briefcase1->GetiModelConnection().DownloadSeedFile(path, fileInfo->GetFileId().ToString())->GetResult();
    EXPECT_SUCCESS(stat);
    }