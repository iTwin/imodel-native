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
        m_client     = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidAdminCredentials(), proxy);
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

    void DeleteiModels()
        {
        auto result = m_client->GetiModels()->GetResult();
        EXPECT_SUCCESS(result);

        for (auto imodel : result.GetValue())
            {
            DeleteiModel(*m_client, *imodel);
            }
        }

    iModelInfoPtr CreateNewiModel()
        {
        return IntegrationTestsBase::CreateNewiModel(*m_client, "ClientTest");
        }
    };

TEST_F(ClientTests, SuccessfulCreateBasicUser)
    {
    if (IntegrationTestSettings::Instance().IsIms())
        return;
    
    Utf8String userName;
    BeSQLite::BeGuid guid;
    guid.Create();
    userName.Sprintf("User%s", guid.ToString().c_str());
    Credentials credentials(userName, "password");
    auto result = m_client->CreateBasicUser(credentials)->GetResult();
    EXPECT_SUCCESS(result);

    //Create non-admin user for a second time
    result = m_client->CreateBasicUser(credentials)->GetResult();
    EXPECT_FALSE(result.IsSuccess());

    auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), credentials);

    //Try to create new user by using non-admin user
    Credentials credentials2("additionalUser", "additionalPass");
    result = nonAdminClient->CreateBasicUser(credentials2)->GetResult();
    EXPECT_FALSE(result.IsSuccess());

    //Try to remove user using non-admin user
    result = nonAdminClient->RemoveBasicUser(credentials)->GetResult();
    EXPECT_FALSE(result.IsSuccess());

    result = m_client->RemoveBasicUser(credentials)->GetResult();
    EXPECT_SUCCESS(result);

    //Try to remove non-admin user for asecond time by using admin user
    result = m_client->RemoveBasicUser(credentials)->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    }

TEST_F(ClientTests, SuccessfulCreateiModel)
    {
    auto db = CreateTestDb();

    auto createResult = m_client->CreateNewiModel(*db, true, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(createResult);
    CheckProgressNotified();

    auto result = m_client->GetiModels()->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_FALSE(result.GetValue().empty());

    DeleteiModel(*m_client, *createResult.GetValue());
    }

TEST_F(ClientTests, UnauthorizedCreateiModel)
    {
    auto db = CreateTestDb();

    m_client = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidNonAdminCredentials());

    auto result = m_client->CreateNewiModel(*db, true, CreateProgressCallback())->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());
    CheckNoProgress();
    }

TEST_F(ClientTests, RepeatedCreateiModel)
    {
    auto db = CreateTestDb();

    auto result = m_client->CreateNewiModel(*db, true, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(result);
    CheckProgressNotified();
    auto imodel = result.GetValue();
    ConnectToiModel(*m_client, imodel);

    result = m_client->CreateNewiModel(*db, true, CreateProgressCallback())->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    CheckNoProgress();
    EXPECT_EQ(Error::Id::iModelAlreadyExists, result.GetError().GetId());
    DeleteiModel(*m_client, *imodel);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             08/2017
//---------------------------------------------------------------------------------------
TEST_F(ClientTests, CancelCreateiModel)
    {
    auto db = CreateTestDb();

    // Act
    SimpleCancellationTokenPtr cancellationToken = SimpleCancellationToken::Create();
    auto createiModelTask = m_client->CreateNewiModel(*db, true, nullptr, cancellationToken);
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
    
    auto iModelToDeleteResult = m_client->GetiModelByName(name)->GetResult();
    if (iModelToDeleteResult.IsSuccess())
        {
        auto iModelToDeletePtr = iModelToDeleteResult.GetValue();
        EXPECT_FALSE(iModelToDeletePtr.IsValid());
        if (iModelToDeletePtr.IsValid())
            {
            DeleteiModel(*m_client, *iModelToDeletePtr);
            }
        }
    }

TEST_F(ClientTests, UnsuccessfulCreateiModel)
    {
    auto db = CreateTestDb();

    m_client = SetUpClient(IntegrationTestSettings::Instance().GetInvalidHost(), IntegrationTestSettings::Instance().GetValidAdminCredentials());

    auto result = m_client->CreateNewiModel(*db, true, CreateProgressCallback())->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(Error::Id::WebServicesError, result.GetError().GetId());
    CheckNoProgress();
    }

TEST_F(ClientTests, SuccessfulGetiModels)
    {
    DeleteiModels();
    auto imodel1 = CreateNewiModel();
    auto imodel2 = CreateNewiModel();

    auto result = m_client->GetiModels()->GetResult();
    EXPECT_SUCCESS(result);

    bvector<iModelInfoPtr>& imodels = result.GetValue();
    EXPECT_EQ(2, imodels.size());
    DateTime compareDate (DateTime::Kind::Utc, 2017, 1, 1, 0, 0, 0, 0);
    for (iModelInfoPtr imodel : imodels)
        {
        EXPECT_FALSE(imodel->GetServerURL().empty());
        EXPECT_FALSE(imodel->GetId().empty());
        EXPECT_FALSE(imodel->GetName().empty());

        DateTimeCR createdDate = imodel->GetCreatedDate();
        EXPECT_TRUE(createdDate.IsValid());
        EXPECT_EQ((int)DateTime::CompareResult::EarlierThan, (int)DateTime::Compare(compareDate, createdDate));
        }
    DeleteiModel(*m_client, *imodel1);
    DeleteiModel(*m_client, *imodel2);
    }

TEST_F(ClientTests, UnsuccessfulGetiModels)
    {
    auto imodel = CreateNewiModel();

    auto badClient = SetUpClient(IntegrationTestSettings::Instance().GetInvalidHost(), IntegrationTestSettings::Instance().GetValidAdminCredentials());

    auto result = badClient->GetiModels()->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(Error::Id::WebServicesError, result.GetError().GetId());
    DeleteiModel(*m_client, *imodel);
    }

TEST_F(ClientTests, EmptyGetiModels)
    {
    DeleteiModels();

    auto result = m_client->GetiModels()->GetResult();

    EXPECT_SUCCESS(result);
    EXPECT_TRUE(result.GetValue().empty());
    }

TEST_F (ClientTests, SuccessfulCreateiModelWithASpaceInName)
    {
    auto db = IntegrationTestsBase::CreateTestDb("Client Test");

    auto createResult = m_client->CreateNewiModel(*db, true, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(createResult);
    CheckProgressNotified();

    auto result = m_client->GetiModels ()->GetResult ();
    EXPECT_SUCCESS(result);
    EXPECT_TRUE (!result.GetValue ().empty ());
    DeleteiModel(*m_client, *createResult.GetValue());
    }

TEST_F(ClientTests, ReplaceSeedFile)
    {
    // Create imodel and get a connection to it
    auto db = CreateTestDb();
    auto firstGuid = db->GetDbGuid();
    auto createResult = m_client->CreateNewiModel(*db, true, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(createResult);
    CheckProgressNotified();
    auto imodelInfoPtr = createResult.GetValue();
    auto connectionResult = m_client->ConnectToiModel(createResult.GetValue()->GetId())->GetResult();
    EXPECT_SUCCESS(connectionResult);
    auto imodelConnection = connectionResult.GetValue();

    // Attempt to create a new seed file with same Guid
    auto fileName = db->GetFileName();
    FileInfoPtr fileInfo = FileInfo::Create(*db, "Replacement description0");
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    auto uploadResult = imodelConnection->UploadNewSeedFile(fileName, *fileInfo, true, CreateProgressCallback())->GetResult();
    EXPECT_FALSE(uploadResult.IsSuccess());
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());
    CheckNoProgress();

    // Replace seed file sending lock imodel request first
    BeSQLite::BeGuid secondGuid;
    secondGuid.Create();
    db->ChangeDbGuid(secondGuid); // Just changing the Guid for test instead of creating new file
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());
    fileInfo = FileInfo::Create(*db, "Replacement description1");
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    EXPECT_SUCCESS(imodelConnection->UploadNewSeedFile(fileName, *fileInfo, true, CreateProgressCallback())->GetResult());
    CheckProgressNotified();

    // Replace seed file without sending a lock request first
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());
    BeSQLite::BeGuid thirdGuid;
    thirdGuid.Create();
    db->ChangeDbGuid(thirdGuid);
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());
    fileInfo = FileInfo::Create(*db, "Replacement description2");
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    EXPECT_SUCCESS(imodelConnection->UploadNewSeedFile(fileName, *fileInfo, true, CreateProgressCallback())->GetResult());
    CheckProgressNotified();
    db->CloseDb();
    
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

    DeleteiModel(*m_client, *createResult.GetValue());
    }

TEST_F(ClientTests, DownloadArchivedFiles)
    {
    // Create imodel and get a connection to it
    auto db = CreateTestDb();
    iModelInfoPtr imodelInfo = IntegrationTestsBase::CreateNewiModelFromDb(*m_client, *db);
    iModelConnectionPtr imodelConnection = IntegrationTestsBase::ConnectToiModel(*m_client, imodelInfo);
    BeSQLite::BeGuid originalGuid = db->GetDbGuid();

    // Add two changeSets to original seed file and add another changeSet after replacing it
    IntegrationTestsBase::InitializeWithChangeSets(*m_client, *imodelInfo, 2);
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

    DeleteiModel(*m_client, *imodelInfo);
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
    DeleteiModel(*m_client, *imodelInfo);
    }
