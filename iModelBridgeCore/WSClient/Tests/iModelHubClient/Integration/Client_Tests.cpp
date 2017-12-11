/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/Client_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include "IntegrationTestsHelper.h"
#include <WebServices/iModelHub/Client/Client.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

struct ClientTests : public IntegrationTestsBase
    {
    ClientPtr    m_client;

    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();
        auto proxy   = ProxyHttpHandler::GetFiddlerProxyIfReachable();
        m_client     = SetUpClient(IntegrationTestSettings::Instance().GetValidAdminCredentials(), proxy);
        m_pHost->SetRepositoryAdmin(m_client->GetiModelAdmin());
        }

    virtual void TearDown() override
        {
        m_client = nullptr;
        IntegrationTestsBase::TearDown();
        }

    DgnDbPtr CreateTestDb()
        {
        return IntegrationTestsBase::CreateTestDb("ClientTest");
        }

    iModelInfoPtr CreateNewiModel()
        {
        return IntegrationTestsBase::CreateNewiModel(*m_client, "ClientTest");
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClientTests, SuccessfulCreateiModel)
    {
    auto db = CreateTestDb();

    auto createResult = m_client->CreateNewiModel(m_projectId, *db, true, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(createResult);
    CheckProgressNotified();

    auto result = m_client->GetiModels(m_projectId)->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_EQ(result.GetValue()[0]->GetUserCreated(), result.GetValue()[0]->GetOwnerInfo()->GetId());
    EXPECT_FALSE(result.GetValue().empty());

    DeleteiModel(m_projectId, *m_client, *createResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClientTests, UnauthorizedCreateiModel)
    {
    auto db = CreateTestDb();

    m_client = SetUpClient(IntegrationTestSettings::Instance().GetValidNonAdminCredentials());

    auto result = m_client->CreateNewiModel(m_projectId, *db, true, CreateProgressCallback())->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());
    CheckNoProgress();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClientTests, RepeatedCreateiModel)
    {
    auto db = CreateTestDb();

    auto result = m_client->CreateNewiModel(m_projectId, *db, true, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(result);
    CheckProgressNotified();
    auto imodel = result.GetValue();
    ConnectToiModel(*m_client, imodel);

    result = m_client->CreateNewiModel(m_projectId, *db, true, CreateProgressCallback())->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    CheckNoProgress();
    EXPECT_EQ(Error::Id::iModelAlreadyExists, result.GetError().GetId());
    DeleteiModel(m_projectId, *m_client, *imodel);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             08/2017
//---------------------------------------------------------------------------------------
TEST_F(ClientTests, CancelCreateiModel)
    {
    auto db = CreateTestDb();

    // Act
    SimpleCancellationTokenPtr cancellationToken = SimpleCancellationToken::Create();
    auto createiModelTask = m_client->CreateNewiModel(m_projectId, *db, true, nullptr, cancellationToken);
    createiModelTask->Execute();
    cancellationToken->SetCanceled();
    createiModelTask->Wait();

    // Assert
    auto cancelledResult = createiModelTask->GetResult();
    EXPECT_FALSE(cancelledResult.IsSuccess());

    // Cleanup
    Utf8String name;
    db->QueryProperty(name, BeSQLite::PropertySpec("Name", "dgn_Proj"));
    if (name.empty())
        BeStringUtilities::WCharToUtf8(name, db->GetFileName().GetFileNameWithoutExtension().c_str());
    
    auto iModelToDeleteResult = m_client->GetiModelByName(m_projectId, name)->GetResult();
    if (iModelToDeleteResult.IsSuccess())
        {
        auto iModelToDeletePtr = iModelToDeleteResult.GetValue();
        EXPECT_FALSE(iModelToDeletePtr.IsValid());
        if (iModelToDeletePtr.IsValid())
            {
            DeleteiModel(m_projectId, *m_client, *iModelToDeletePtr);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClientTests, UnsuccessfulCreateiModel)
    {
    auto db = CreateTestDb();

    m_client = SetUpClient(IntegrationTestSettings::Instance().GetValidAdminCredentials());

    auto result = m_client->CreateNewiModel("bad project", *db, true, CreateProgressCallback())->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(Error::Id::FailedToGetProjectPermissions, result.GetError().GetId());
    CheckNoProgress();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClientTests, UnsuccessfulGetiModels)
    {
    auto imodel = CreateNewiModel();

    auto badClient = SetUpClient(IntegrationTestSettings::Instance().GetValidAdminCredentials());

    auto result = badClient->GetiModels("bad project")->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(Error::Id::FailedToGetProjectPermissions, result.GetError().GetId());
    DeleteiModel(m_projectId, *m_client, *imodel);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClientTests, SuccessfulCreateiModelWithASpaceInName)
    {
    auto db = IntegrationTestsBase::CreateTestDb("Client Test");

    auto createResult = m_client->CreateNewiModel(m_projectId, *db, true, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(createResult);
    CheckProgressNotified();

    auto result = m_client->GetiModels (m_projectId)->GetResult ();
    EXPECT_SUCCESS(result);
    EXPECT_EQ(result.GetValue()[0]->GetUserCreated(), result.GetValue()[0]->GetOwnerInfo()->GetId());
    EXPECT_TRUE (!result.GetValue ().empty ());
    DeleteiModel(m_projectId, *m_client, *createResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClientTests, ReplaceSeedFile)
    {
    // Create imodel and get a connection to it
    auto db = CreateTestDb();
    auto firstGuid = db->GetDbGuid();
    auto createResult = m_client->CreateNewiModel(m_projectId, *db, true, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(createResult);
    CheckProgressNotified();
    auto imodelInfoPtr = createResult.GetValue();
    auto connectionResult = m_client->ConnectToiModel(*createResult.GetValue())->GetResult();
    EXPECT_SUCCESS(connectionResult);
    auto imodelConnection = connectionResult.GetValue();


    // Replace seed file sending lock imodel request first
    auto fileName = db->GetFileName();
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());
    FileInfoPtr fileInfo = FileInfo::Create(*db, "Replacement description1");
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    EXPECT_SUCCESS(imodelConnection->UploadNewSeedFile(fileName, *fileInfo, true, CreateProgressCallback())->GetResult());
    CheckProgressNotified();

    auto seedFileResult = imodelConnection->GetLatestSeedFile()->GetResult();
    EXPECT_SUCCESS(seedFileResult);
    auto secondGuid = seedFileResult.GetValue()->GetFileId();
    EXPECT_NE(firstGuid, secondGuid);

    // Replace seed file without sending a lock request first
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());
    fileInfo = FileInfo::Create(*db, "Replacement description2");
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    EXPECT_SUCCESS(imodelConnection->UploadNewSeedFile(fileName, *fileInfo, true, CreateProgressCallback())->GetResult());
    CheckProgressNotified();
    db->CloseDb();

    seedFileResult = imodelConnection->GetLatestSeedFile()->GetResult();
    EXPECT_SUCCESS(seedFileResult);
    auto thirdGuid = seedFileResult.GetValue()->GetFileId();
    EXPECT_NE(thirdGuid, secondGuid);
    EXPECT_NE(thirdGuid, firstGuid);
    
    // Acquire first briefcase
    auto newBriefcase = IntegrationTestsBase::AcquireBriefcase(*m_client, *imodelInfoPtr);
    DgnDbR newdb = newBriefcase->GetDgnDb();
    auto model1 = CreateModel("Model1", newdb);
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, newdb.SaveChanges());
    auto pushResult = newBriefcase->PullMergeAndPush(nullptr, false, nullptr, CreateProgressCallback())->GetResult();
    CheckProgressNotified();
    EXPECT_SUCCESS(pushResult);

    // Acquire one more briefcase
    auto newBriefcase2 = IntegrationTestsBase::AcquireBriefcase(*m_client, *imodelInfoPtr);
    pushResult = newBriefcase2->PullAndMerge(CreateProgressCallback())->GetResult();

    DgnDbR newdb2 = newBriefcase2->GetDgnDb();
    auto model2 = CreateModel("Model2", newdb2);
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, newdb2.SaveChanges());
    pushResult = newBriefcase2->PullMergeAndPush(nullptr, false, nullptr, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(pushResult);
    CheckProgressNotified();

    DeleteiModel(m_projectId, *m_client, *createResult.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClientTests, DownloadArchivedFiles)
    {
    // Create imodel and get a connection to it
    auto db = CreateTestDb();
    iModelInfoPtr imodelInfo = IntegrationTestsBase::CreateNewiModelFromDb(*m_client, *db);
    iModelConnectionPtr imodelConnection = IntegrationTestsBase::ConnectToiModel(*m_client, imodelInfo);
    
    // Add two changeSets to original seed file and add another changeSet after replacing it
    BriefcasePtr briefcase1 = IntegrationTestsBase::InitializeWithChangeSets(*m_client, *imodelInfo, 2);
    auto originalGuid = briefcase1->GetDgnDb().GetDbGuid();
    IntegrationTestsBase::ReplaceSeedFile(imodelConnection, *db);
    IntegrationTestsBase::InitializeWithChangeSets(*m_client, *imodelInfo, 1);

    // Query available seed files
    auto seedFilesResult = imodelConnection->GetSeedFiles()->GetResult();
    EXPECT_SUCCESS(seedFilesResult);
    auto seedFiles = seedFilesResult.GetValue();
    EXPECT_EQ(2, seedFiles.size());
    auto originalFile = originalGuid == seedFiles[0]->GetFileId() ? seedFiles[0] : seedFiles[1];
    EXPECT_EQ(originalGuid, originalFile->GetFileId());
    
    // Download old seed file
    BeFileName originalFilePath = IntegrationTestsBase::LocalPath("OriginalFile");
    auto downloadResult = imodelConnection->DownloadSeedFile(originalFilePath, originalFile->GetFileId().ToString(), CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(downloadResult);
    CheckProgressNotified();
    DgnDbPtr originalDb = DgnDb::OpenDgnDb(nullptr, originalFilePath, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(originalDb.IsValid());
    EXPECT_EQ(originalGuid, originalDb->GetDbGuid());

    // Download available changeSets
    auto changeSetsResult = imodelConnection->DownloadChangeSetsAfterId("", originalGuid, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    CheckProgressNotified();
    auto changeSets = changeSetsResult.GetValue();
    EXPECT_EQ(2, changeSets.size());

    // Attempt to merge the changeSets
    originalDb->SetAsBriefcase(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
    originalDb->SaveChanges();
    originalDb->CloseDb();
    originalDb = DgnDb::OpenDgnDb(nullptr, originalFilePath, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    for (auto const& changeSet : changeSets)
        {
        RevisionStatus status = originalDb->Revisions().MergeRevision(*changeSet);
        EXPECT_EQ(RevisionStatus::Success, status);
        }
    originalDb->CloseDb();

    DeleteiModel(m_projectId, *m_client, *imodelInfo);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             08/2017
//---------------------------------------------------------------------------------------
TEST_F(ClientTests, CancelDownloadChangeSets)
    {
    // Create imodel and get a connection to it
    auto db = CreateTestDb();
    iModelInfoPtr imodelInfo = IntegrationTestsBase::CreateNewiModelFromDb(*m_client, *db);
    iModelConnectionPtr imodelConnection = IntegrationTestsBase::ConnectToiModel(*m_client, imodelInfo);
    BeSQLite::BeGuid originalGuid = db->GetDbGuid();

    IntegrationTestsBase::InitializeWithChangeSets(*m_client, *imodelInfo, 5);

    // Act
    SimpleCancellationTokenPtr cancellationToken = SimpleCancellationToken::Create();
    auto changeSetsTask = imodelConnection->DownloadChangeSetsAfterId("", originalGuid, CreateProgressCallback(), cancellationToken);
    changeSetsTask->Execute();
    cancellationToken->SetCanceled();

    changeSetsTask->Wait();
    
    EXPECT_TRUE(changeSetsTask->IsCompleted());
    EXPECT_FALSE(changeSetsTask->GetResult().IsSuccess());

    CheckNoProgress();
    DeleteiModel(m_projectId, *m_client, *imodelInfo);
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClientTests, UnauthorizedSignIn)
    {
    auto badClient = SetUpClient(IntegrationTestSettings::Instance().GetWrongPassword());
    EXPECT_TRUE(badClient.IsNull());
    }
