/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/PushRetryTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"
#include "../../../Client/ServerInfoProvider.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct PushRetryTests : public iModelTestsBase
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
* @bsimethod                                    Algirdas.Mikoliunas             09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PushRetryTests, QueryChangeSetsFails)
    {
    //Create model in m_briefcase. This should also acquire locks automatically.
    PhysicalModelPtr model = CreateModel (TestCodeName().c_str(), m_briefcase->GetDgnDb());

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(3)
        .ForRequest(1, SuccessResponse)
        .ForRequest(2, TimeoutResponse) // Query changeSet instances fails
        .ForRequest(3, [=](Http::RequestCR request)
            {
            // Query changeSet instances fails
            m_connection->SetRepositoryClient(m_originalClient);
            return TimeoutResponse(request);
            });
    
    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);
    
    // First push should fail
    ChangeSetsResult pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    ASSERT_FAILURE(pushResult);
    
    // Second push should succeed
    pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PushRetryTests, QueryChangeSetsFailsSingleTime)
    {
    //Create model in m_briefcase. This should also acquire locks automatically.
    PhysicalModelPtr model = CreateModel (TestCodeName().c_str(), m_briefcase->GetDgnDb());

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(2)
        .ForRequest(1, SuccessResponse)
        .ForRequest(2, [=](Http::RequestCR request)
            {
            // Query changeSet instances fails
            m_connection->SetRepositoryClient(m_originalClient);
            return TimeoutResponse(request);
            });
    
    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);
    
    // First push should fail
    ChangeSetsResult pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PushRetryTests, CreateChangeSetFails)
    {
    //Create model in m_briefcase. This should also acquire locks automatically.
    PhysicalModelPtr model = CreateModel(TestCodeName().c_str(), m_briefcase->GetDgnDb());

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(4)
        .ForRequest(1, EmptyInstancesResponse) // Query changeSet instances
        .ForRequest(2, ServerErrorResponse) // Create changeSet in the server fails
        .ForRequest(3, SuccessResponse) // Plugins request
        .ForRequest(4, [=](Http::RequestCR request)
            {
            // Create changeSet in the server fails
            m_connection->SetRepositoryClient(m_originalClient);
            return ServerErrorResponse(request);
            });
    
    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);

    // First push should fail
    ChangeSetsResult pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    ASSERT_FAILURE(pushResult);

    // Second push should succeed
    pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Http::Response XmlTimeoutResponse(Http::RequestCR request)
    {
    Utf8CP body = "<?xml version=\"1.0\" encoding=\"utf-8\"?><Error><Code>RequestTimedOut</Code><Message>TestMessage</Message></Error>";
    return StubHttpResponse(HttpStatus::ReqestTimeout, body, {{ "Content-Type" , REQUESTHEADER_ContentType_ApplicationXml }});
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PushRetryTests, UploadFileFails)
    {
    //Create model in m_briefcase. This should also acquire locks automatically.
    PhysicalModelPtr model = CreateModel(TestCodeName().c_str(), m_briefcase->GetDgnDb());

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(3)
        .ForRequest(1, EmptyInstancesResponse) // Query changeSet instances
        .ForRequest(2, CreatedResponse) // Create changeSet in server
        .ForRequest(3, CreatedResponse); // Create changeSet in server

    IAzureBlobStorageClientPtr oldBlobStorageClient = m_connection->GetAzureBlobStorageClient();
    std::shared_ptr<MockHttpHandler> blobStorageMockHandler = std::make_shared<MockHttpHandler>();
    blobStorageMockHandler->
        ExpectRequests(2)
        .ForRequest(1, XmlTimeoutResponse) // Upload file
        .ForRequest(2, [=](Http::RequestCR request)
            {
            m_connection->SetRepositoryClient(m_originalClient);
            m_connection->SetAzureBlobStorageClient(oldBlobStorageClient);
            return XmlTimeoutResponse(request);
            });
    
    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);
    IAzureBlobStorageClientPtr newAzureClient = iModelHubHelpers::CreateAzureClient(blobStorageMockHandler);
    m_connection->SetAzureBlobStorageClient(newAzureClient);

    // First push should fail
    ChangeSetsResult pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    ASSERT_FAILURE(pushResult);
    EXPECT_EQ("RequestTimedOut", pushResult.GetError().GetMessage());
    EXPECT_EQ("TestMessage", pushResult.GetError().GetDescription());

    // Second push should succeed
    pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PushRetryTests, ConfirmUploadFails)
    {
    //Create model in m_briefcase. This should also acquire locks automatically.
    PhysicalModelPtr model = CreateModel(TestCodeName().c_str(), m_briefcase->GetDgnDb());

    IAzureBlobStorageClientPtr oldBlobStorageClient = m_connection->GetAzureBlobStorageClient();

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(6)
        .ForRequest(1, EmptyInstancesResponse) // Query changeSet instances
        .ForRequest(2, CreatedResponse) // Create changeSet in server
        .ForRequest(3, BadRequestResponse) // Confirm and send locks/codes
        .ForRequest(4, SuccessResponse)
        .ForRequest(5, CreatedResponse) // Create changeSet in server
        .ForRequest(6, [=](Http::RequestCR request)
            {
            // Confirm and send locks/codes
            m_connection->SetRepositoryClient(m_originalClient);
            m_connection->SetAzureBlobStorageClient(oldBlobStorageClient);
            return BadRequestResponse(request);
            });

    std::shared_ptr<MockHttpHandler> blobStorageMockHandler = std::make_shared<MockHttpHandler>();
    blobStorageMockHandler->
        ExpectRequests(2)
        .ForRequest(1, SuccessResponse) // Upload file
        .ForRequest(2, SuccessResponse); // Upload file

    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);
    IAzureBlobStorageClientPtr newAzureClient = iModelHubHelpers::CreateAzureClient(blobStorageMockHandler);
    m_connection->SetAzureBlobStorageClient(newAzureClient);

    // First push should fail
    ChangeSetsResult pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    ASSERT_FAILURE(pushResult);

    // Second push should succeed
    pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PushRetryTests, DownloadedChangeSetInvalid)
    {
    //Create model in m_briefcase. This should also acquire locks automatically.
    PhysicalModelPtr model = CreateModel(TestCodeName().c_str(), m_briefcase->GetDgnDb());

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->
        ExpectRequests(1)
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
                "}, \"relationshipInstances\": [{"
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
                "}]}]}");
            });

    IAzureBlobStorageClientPtr oldBlobStorageClient = m_connection->GetAzureBlobStorageClient();
    std::shared_ptr<MockHttpHandler> blobStorageMockHandler = std::make_shared<MockHttpHandler>();
    blobStorageMockHandler->
        ExpectRequests(1)
        .ForRequest(1, [=](Http::RequestCR request)
            {
            // Download changeSet
            m_connection->SetRepositoryClient(m_originalClient);
            m_connection->SetAzureBlobStorageClient(oldBlobStorageClient);
            return StubHttpResponse(HttpStatus::OK, "Test");
            });
    
    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);
    IAzureBlobStorageClientPtr newAzureClient = iModelHubHelpers::CreateAzureClient(blobStorageMockHandler);
    m_connection->SetAzureBlobStorageClient(newAzureClient);

    // First push should fail
    BeTest::SetFailOnAssert(false);
    ChangeSetsResult pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    BeTest::SetFailOnAssert(true);

    ASSERT_FAILURE(pushResult);
    EXPECT_EQ(Error::Id::ChangeSetManagerError, pushResult.GetError().GetId());

    // Second push should succeed
    pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PushRetryTests, CreateChangeSetTimeoutsButSucceedsOnServer)
    {
    //Create model in m_briefcase. This should also acquire locks automatically.
    PhysicalModelPtr model = CreateModel(TestCodeName().c_str(), m_briefcase->GetDgnDb());
    IHttpHandlerPtr orginalHandler = m_connection->GetHttpHandler();

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->ForAnyRequest([=](Http::RequestCR request)
        {
        Http::Response response = orginalHandler->_PerformRequest(request)->GetResult();
        if (request.GetUrl().EndsWith("ChangeSet"))
            {
            m_connection->SetRepositoryClient(m_originalClient);
            return TimeoutResponse(request);
            }
        return response;
        });
    
    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);

    ChangeSetsResult pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PushRetryTests, ConfirmChangeSetTimeoutsButSucceedsOnServer)
    {
    //Create model in m_briefcase. This should also acquire locks automatically.
    PhysicalModelPtr model = CreateModel(TestCodeName().c_str(), m_briefcase->GetDgnDb());
    IHttpHandlerPtr orginalHandler = m_connection->GetHttpHandler();

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->ForAnyRequest([=](Http::RequestCR request)
        {
        Http::Response response = orginalHandler->_PerformRequest(request)->GetResult();
        if (request.GetUrl().EndsWith("$changeset"))
            {
            m_connection->SetRepositoryClient(m_originalClient);
            return TimeoutResponse(request);
            }
        return response;
        });
    
    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);

    ChangeSetsResult pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PushRetryTests, CodesLocksPushAfterChangesetCreatedTimeouts)
    {
    //Create model in m_briefcase. This should also acquire locks automatically.
    PhysicalModelPtr model = CreateModel(TestCodeName().c_str(), m_briefcase->GetDgnDb());
    IHttpHandlerPtr orginalHandler = m_connection->GetHttpHandler();

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->ForAnyRequest([=](Http::RequestCR request)
        {
        auto requestBody = request.GetRequestBody();
        if (!requestBody.IsNull() && requestBody->AsString().ContainsI("ConflictStrategy"))
            {
            return ServerErrorResponse(request);
            }

        return orginalHandler->_PerformRequest(request)->GetResult();
        });
    
    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);

    // Push changeset, codes/locks push fails
    ChangeSetsResult pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);

    auto imodelManager1 = IntegrationTestsBase::_GetRepositoryManager(m_briefcase->GetDgnDb());
    ExpectCodeState(CreateCodeReserved(model->GetModeledElement()->GetCode(), m_briefcase->GetDgnDb()), imodelManager1);

    // Set back to normal client
    m_connection->SetRepositoryClient(m_originalClient);

    // Second PullMergeAndPush should upload pending codes/locks
    pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    ExpectCodeState(CreateCodeUsed(model->GetModeledElement()->GetCode(), m_briefcase->GetLastChangeSetPulled()), imodelManager1);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas             04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PushRetryTests, CodesLocksArePushedWithThirdPullMergeAndPush)
    {
    //Create model in m_briefcase. This should also acquire locks automatically.
    PhysicalModelPtr model = CreateModel(TestCodeName().c_str(), m_briefcase->GetDgnDb());
    IHttpHandlerPtr orginalHandler = m_connection->GetHttpHandler();

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();
    mockHandler->ForAnyRequest([=](Http::RequestCR request)
        {
        auto requestBody = request.GetRequestBody();
        if (!requestBody.IsNull() && requestBody->AsString().ContainsI("ConflictStrategy"))
            {
            return ServerErrorResponse(request);
            }

        return orginalHandler->_PerformRequest(request)->GetResult();
        });
    
    // Set other wsclient
    IWSRepositoryClientPtr newClient = iModelHubHelpers::CreateWSClient(s_info, mockHandler);
    m_connection->SetRepositoryClient(newClient);

    // Push changeset, codes/locks push fails
    ChangeSetsResult pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);

    auto imodelManager1 = IntegrationTestsBase::_GetRepositoryManager(m_briefcase->GetDgnDb());
    ExpectCodeState(CreateCodeReserved(model->GetModeledElement()->GetCode(), m_briefcase->GetDgnDb()), imodelManager1);

    // Do some more changes to iModel
    PhysicalModelPtr model2 = CreateModel(TestCodeName(2).c_str(), m_briefcase->GetDgnDb());

    // Second PullMergeAndPush codes/locks push also fails
    pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);

    // Check both changesets codes are still reserved
    ExpectCodeState(CreateCodeReserved(model->GetModeledElement()->GetCode(), m_briefcase->GetDgnDb()), imodelManager1);
    ExpectCodeState(CreateCodeReserved(model2->GetModeledElement()->GetCode(), m_briefcase->GetDgnDb()), imodelManager1);

    // Set back to normal client
    m_connection->SetRepositoryClient(m_originalClient);

    // Second PullMergeAndPush should uploads pending codes/locks
    pushResult = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    EXPECT_SUCCESS(pushResult);
    ExpectCodeState(CreateCodeUsed(model->GetModeledElement()->GetCode(), m_briefcase->GetLastChangeSetPulled()), imodelManager1);
    ExpectCodeState(CreateCodeUsed(model2->GetModeledElement()->GetCode(), m_briefcase->GetLastChangeSetPulled()), imodelManager1);
    }
