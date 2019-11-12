/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"
#include "../../../Client/ServerInfoProvider.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct RetryTests : public iModelTestsBase
    {
    BriefcasePtr m_briefcase;
    iModelConnectionPtr m_connection;
    IWSRepositoryClientPtr m_originalClient;

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void SetUp() override
        {
        iModelTestsBase::SetUp();
        m_briefcase = AcquireAndOpenBriefcase();
        m_connection = m_briefcase->GetiModelConnectionPtr();
        m_originalClient = m_connection->GetRepositoryClient();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void TearDown() override
        {
        iModelManager* manager = (iModelManager*)_GetRepositoryManager(m_briefcase->GetDgnDb());
        manager->GetiModelConnectionPtr()->SetRepositoryClient(m_originalClient);
        if (m_briefcase.IsValid())
            {
            m_briefcase = nullptr;
            }
        if (m_connection.IsValid())
            {
            m_connection = nullptr;
            }
        if (nullptr != m_originalClient)
            {
            m_originalClient = nullptr;
            }
        iModelTestsBase::TearDown();
        }
    };


/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RetryTests, DownloadSeedFileRetries)
    {
    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(1)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::OK, "{\"instances\":[{\"schemaName\": \"iModelScope\", \"className\": \"SeedFile\", \"instanceId\" : \"Id\", \"properties\":{},"
                "\"relationshipInstances\": [{"
                        "\"instanceId\": \"\","
                            "\"schemaName\" : \"iModelScope\","
                            "\"className\" : \"FileAccessKey\","
                            "\"direction\" : \"forward\","
                            "\"properties\" : {},"
                            "\"relatedInstance\" : {"
                            "\"instanceId\": \"\","
                                "\"schemaName\" : \"iModelScope\","
                                "\"className\" : \"AccessKey\","
                                "\"properties\" : {"
                                    "\"UploadUrl\": \"\","
                                    "\"DownloadUrl\" : \"http://download.url\""
                                "},"
                                "\"eTag\" : \"GDZV8h4lHbFl9QQEPaMCf4WLZVE=\""
                        "},"
                      "\"eTag\": \"X4OhiRIhgh0VXBSbUPbs6D3dZvc=\""
                    "}]}]}");});

    std::shared_ptr<MockHttpHandler> blobStorageMockHandler = std::make_shared<MockHttpHandler>();
    blobStorageMockHandler->
        ExpectRequests(2)
        .ForRequest(1, TimeoutResponse) // Download seed file
        .ForRequest(2, TimeoutResponse);

    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);
    IAzureBlobStorageClientPtr newAzureClient = iModelHubHelpers::CreateAzureClient(blobStorageMockHandler);
    m_connection->SetAzureBlobStorageClient(newAzureClient);

    StatusResult statusResult = m_briefcase->GetiModelConnection().DownloadSeedFile(OutputDir(), m_briefcase->GetDgnDb().GetDbGuid().ToString())->GetResult();
    EXPECT_FAILURE(statusResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RetryTests, DownloadSeedFileSingleFailSucceeds)
    {
    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(1)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            return StubHttpResponse(HttpStatus::OK, "{\"instances\":[{\"schemaName\": \"iModelScope\", \"className\": \"SeedFile\", \"instanceId\" : \"Id\", \"properties\":{},"
                "\"relationshipInstances\": [{"
                        "\"instanceId\": \"\","
                            "\"schemaName\" : \"iModelScope\","
                            "\"className\" : \"FileAccessKey\","
                            "\"direction\" : \"forward\","
                            "\"properties\" : {},"
                            "\"relatedInstance\" : {"
                            "\"instanceId\": \"\","
                                "\"schemaName\" : \"iModelScope\","
                                "\"className\" : \"AccessKey\","
                                "\"properties\" : {"
                                    "\"UploadUrl\": \"\","
                                    "\"DownloadUrl\" : \"http://download.url\""
                                "},"
                                "\"eTag\" : \"GDZV8h4lHbFl9QQEPaMCf4WLZVE=\""
                        "},"
                      "\"eTag\": \"X4OhiRIhgh0VXBSbUPbs6D3dZvc=\""
                    "}]}]}");});

    std::shared_ptr<MockHttpHandler> blobStorageMockHandler = std::make_shared<MockHttpHandler>();
    blobStorageMockHandler->
        ExpectRequests(2)
        .ForRequest(1, TimeoutResponse) // Download seed file
        .ForRequest(2, [=](Http::RequestCR request)
            {
            // Second download succeeds
            return StubHttpResponse(HttpStatus::OK, "Content");
            }
        );

    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);
    IAzureBlobStorageClientPtr newAzureClient = iModelHubHelpers::CreateAzureClient(blobStorageMockHandler);
    m_connection->SetAzureBlobStorageClient(newAzureClient);

    BeFileName path = OutputDir().AppendToPath(L"SeedFileRetry.bim");
    StatusResult statusResult = m_briefcase->GetiModelConnection().DownloadSeedFile(OutputDir(), m_briefcase->GetDgnDb().GetDbGuid().ToString())->GetResult();
    EXPECT_SUCCESS(statusResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RetryTests, AcquireCodesLocksFails)
    {
    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(6)
        .ForRequest(1, EmptyInstancesResponse) // Query held resources
        .ForRequest(2, EmptyInstancesResponse) // Query unavailable locks/codes
        .ForRequest(3, EmptyInstancesResponse) // Query unavailable locks/codes
        .ForRequest(4, EmptyInstancesResponse) // Query unavailable locks/codes
        .ForRequest(5, TimeoutResponse)
        .ForRequest(6, TimeoutResponse);

    DgnDbR db = m_briefcase->GetDgnDb();
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), TestCodeName());
    EXPECT_TRUE(partition.IsValid());

    // Set other wsclient
    iModelManager* manager = (iModelManager*)_GetRepositoryManager(db);
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    manager->GetiModelConnectionPtr()->SetRepositoryClient(newClient);

    RepositoryStatus repositoryStatus = db.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::ServerUnavailable, repositoryStatus);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RetryTests, AcquireCodesLocksSingleFailSucceeds)
    {
    DgnDbR db = m_briefcase->GetDgnDb();
    iModelManager* manager = (iModelManager*)_GetRepositoryManager(db);

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(5)
        .ForRequest(1, SuccessResponse) 
        .ForRequest(2, EmptyInstancesResponse) // Query held resources
        .ForRequest(3, EmptyInstancesResponse) // Query unavailable locks/codes
        .ForRequest(4, EmptyInstancesResponse) // Query unavailable locks/codes
        .ForRequest(5,  [=](Http::RequestCR request)
            {
            manager->GetiModelConnectionPtr()->SetRepositoryClient(m_originalClient);
            return TimeoutResponse(request);
            }
        );

    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), TestCodeName());
    EXPECT_TRUE(partition.IsValid());

    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    manager->GetiModelConnectionPtr()->SetRepositoryClient(newClient);

    RepositoryStatus repositoryStatus = db.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::Success, repositoryStatus);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RetryTests, AcquireCodesLocksQueryHeldResourcesFails)
    {
    DgnDbR db = m_briefcase->GetDgnDb();
    iModelManager* manager = (iModelManager*)_GetRepositoryManager(db);

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(8)
        .ForRequest(1, TimeoutResponse) // iModelScope/MultiCode?$filter=(BriefcaseId+eq+2)
        .ForRequest(2, TimeoutResponse) // iModelScope/MultiCode?$filter=(BriefcaseId+eq+2)
        .ForRequest(3, TimeoutResponse) // iModelScope/MultiLock?$filter=(BriefcaseId+eq+2)
        .ForRequest(4, TimeoutResponse) // iModelScope/MultiLock?$filter=(BriefcaseId+eq+2)
        .ForRequest(5, TimeoutResponse) // iModelScope/Code?$filter=BriefcaseId+ne+2
        .ForRequest(6, TimeoutResponse) // iModelScope/Code?$filter=BriefcaseId+ne+2
        .ForRequest(7, TimeoutResponse) // iModelScope/Lock?$filter=BriefcaseId+ne+2+and+(LockLevel+gt+0+or+ReleasedWithChangeSetIndex+gt+0)
        .ForRequest(8,  [=](Http::RequestCR request)
            {
            // iModelScope/Lock?$filter=BriefcaseId+ne+2+and+(LockLevel+gt+0+or+ReleasedWithChangeSetIndex+gt+0)
            manager->GetiModelConnectionPtr()->SetRepositoryClient(m_originalClient);
            return TimeoutResponse(request);
            }
        );

    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), TestCodeName());
    EXPECT_TRUE(partition.IsValid());

    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    manager->GetiModelConnectionPtr()->SetRepositoryClient(newClient);

    RepositoryStatus repositoryStatus = db.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::ServerUnavailable, repositoryStatus);
    }
