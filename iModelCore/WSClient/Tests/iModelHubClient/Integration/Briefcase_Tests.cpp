/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/Briefcase_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include "IntegrationTestsHelper.h"
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/TxnManager.h>
#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/iModelHub/Client/Configuration.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

struct BriefcaseTests: public IntegrationTestsBase
    {
    ClientPtr    m_client;
    iModelInfoPtr m_imodel;
    iModelConnectionPtr m_imodelConnection;

    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();

        auto proxy   = ProxyHttpHandler::GetFiddlerProxyIfReachable();
        m_client     = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidAdminCredentials(), proxy);
        m_imodel = CreateNewiModel(*m_client, "BriefcaseTest");

        m_imodelConnection = ConnectToiModel(*m_client, m_imodel);

        m_pHost->SetRepositoryAdmin(m_client->GetiModelAdmin());
        }

    virtual void TearDown() override
        {
        if (m_imodel.IsValid())
            DeleteiModel(*m_client, *m_imodel);
        m_client = nullptr;
        IntegrationTestsBase::TearDown();
        }

    BriefcasePtr AcquireBriefcase()
        {
        return IntegrationTestsBase::AcquireBriefcase(*m_client, *m_imodel);
        }

    DgnDbPtr OpenBriefcaseFile()
        {
        auto result = m_client->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory(), false)->GetResult();

        EXPECT_SUCCESS(result);
        auto dbPath = result.GetValue()->GetLocalPath();
        EXPECT_TRUE(dbPath.DoesPathExist());

        auto db = DgnDb::OpenDgnDb(nullptr, dbPath, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        EXPECT_TRUE(db.IsValid());
        return db;
        }

    void InitializeWithChangeSets()
        {
        IntegrationTestsBase::InitializeWithChangeSets(*m_client, *m_imodel, 2);
        }
    };

TEST_F(BriefcaseTests, SuccessfulAcquireBriefcase)
    {
    auto result = m_client->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory(), false, Client::DefaultFileNameCallback, CreateProgressCallback())->GetResult();

    EXPECT_SUCCESS(result);
    auto dbPath = result.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    CheckProgressNotified();
    }

TEST_F(BriefcaseTests, UnsuccessfulAcquireBriefcase)
    {
    //Attempt acquiring briefcase from a non-existing imodel
    auto imodel = CreateNewiModel(*m_client, "BriefcaseTest");
    auto deleteResult = m_client->DeleteiModel(*imodel)->GetResult();
    EXPECT_SUCCESS(deleteResult);
    auto result = m_client->AcquireBriefcase(*imodel, m_pHost->GetOutputDirectory(), false, CreateProgressCallback())->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(Error::Id::iModelDoesNotExist, result.GetError().GetId());
    CheckNoProgress();
    }

TEST_F(BriefcaseTests, UnauthorizedAcquireBriefcase)
    {
    if (IntegrationTestSettings::Instance().IsIms())
        return;

    auto badClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetWrongPassword());
    auto result = badClient->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory(), false, Client::DefaultFileNameCallback, CreateProgressCallback())->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(Error::Id::LoginFailed, result.GetError().GetId());
    CheckNoProgress();
    }

TEST_F(BriefcaseTests, AcquireAfterQuerying)
    {
    auto imodelResult = m_client->GetiModelById(m_imodel->GetId())->GetResult();
    EXPECT_SUCCESS(imodelResult);
    EXPECT_EQ(imodelResult.GetValue()->GetUserCreated(), imodelResult.GetValue()->GetOwnerInfo()->GetId());

    auto result = m_client->AcquireBriefcaseToDir(*imodelResult.GetValue(), m_pHost->GetOutputDirectory(), false, Client::DefaultFileNameCallback, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(result);
    CheckProgressNotified();

    auto imodelResult2 = m_client->GetiModelByName(m_imodel->GetName())->GetResult();
    EXPECT_SUCCESS(imodelResult2);
    EXPECT_EQ(imodelResult2.GetValue()->GetUserCreated(), imodelResult2.GetValue()->GetOwnerInfo()->GetId());

    auto result2 = m_client->AcquireBriefcaseToDir(*imodelResult2.GetValue(), m_pHost->GetOutputDirectory(), false, Client::DefaultFileNameCallback, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(result2);
    CheckProgressNotified();
    }

TEST_F(BriefcaseTests, AcquireBriefcaseId)
    {
    auto connectionResult = m_client->ConnectToiModel(*m_imodel)->GetResult();
    EXPECT_SUCCESS(connectionResult);
    auto connection = connectionResult.GetValue();

    auto briefcaseResult = connection->AcquireNewBriefcase()->GetResult();
    EXPECT_SUCCESS(briefcaseResult);
    EXPECT_EQ(BeSQLite::BeBriefcaseId(2), briefcaseResult.GetValue()->GetId());

    briefcaseResult = connection->AcquireNewBriefcase()->GetResult();
    EXPECT_SUCCESS(briefcaseResult);
    EXPECT_EQ(BeSQLite::BeBriefcaseId(3), briefcaseResult.GetValue()->GetId());
    }

TEST_F(BriefcaseTests, SuccessfulAbandonBriefcase)
    {
    //Acquire first briefcase
    auto result = m_client->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory(), false, Client::DefaultFileNameCallback, CreateProgressCallback())->GetResult();

    EXPECT_SUCCESS(result);
    auto dbPath = result.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    CheckProgressNotified();

    //Acquire second briefcase
    auto result2 = m_client->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory(), false, Client::DefaultFileNameCallback, CreateProgressCallback())->GetResult();

    EXPECT_SUCCESS(result2);
    dbPath = result2.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    CheckProgressNotified();

    //Abandon briefcase that doesn exists
    auto result3 = m_client->AbandonBriefcase(*m_imodel, BeSQLite::BeBriefcaseId(100))->GetResult();
    EXPECT_FALSE(result3.IsSuccess());

    //Abandon existing briefcase
    auto result4 = m_client->AbandonBriefcase(*m_imodel, result.GetValue()->GetId())->GetResult();
    EXPECT_SUCCESS(result4);

    //Check briefcases by querying
    auto connectResults = m_client->ConnectToiModel(*m_imodel)->GetResult();
    EXPECT_SUCCESS(connectResults);
    auto connection = connectResults.GetValue();
    EXPECT_FALSE(connection->QueryBriefcaseInfo(result.GetValue()->GetId())->GetResult().GetValue().IsValid());
    EXPECT_SUCCESS(connection->QueryBriefcaseInfo(result2.GetValue()->GetId())->GetResult());
    }

TEST_F(BriefcaseTests, SuccessfulAbandonOtherUserBriefcase)
    {
    IntegrationTestsBase::CreateNonAdminUser();
    auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidNonAdminCredentials());

    //User A acquires a briefcase
    auto briefcaseA = m_client->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory(), false, Client::DefaultFileNameCallback, CreateProgressCallback())->GetResult();

    EXPECT_SUCCESS(briefcaseA);
    auto dbPath = briefcaseA.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    CheckProgressNotified();

    //User B acquires a briefcases
    auto briefcaseB = nonAdminClient->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory(), false, Client::DefaultFileNameCallback, CreateProgressCallback())->GetResult();

    EXPECT_SUCCESS(briefcaseB);
    dbPath = briefcaseB.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    CheckProgressNotified();

    //User B should NOT be able to abandon user's A briefcase
    auto result = nonAdminClient->AbandonBriefcase(*m_imodel, briefcaseA.GetValue()->GetId())->GetResult();
    EXPECT_FALSE(result.IsSuccess());

    //User A should be able to abandon user's B briefcase
    result = m_client->AbandonBriefcase(*m_imodel, briefcaseB.GetValue()->GetId())->GetResult();
    EXPECT_SUCCESS(result);

    //User A should NOT be able to abandon not existing briefcase
    result = m_client->AbandonBriefcase(*m_imodel, briefcaseB.GetValue()->GetId())->GetResult();
    EXPECT_FALSE(result.IsSuccess());

    //Check briefcases by querying
    auto connectResults = m_client->ConnectToiModel(*m_imodel)->GetResult();
    EXPECT_SUCCESS(connectResults);
    auto connection = connectResults.GetValue();
    EXPECT_FALSE(connection->QueryBriefcaseInfo(briefcaseB.GetValue()->GetId())->GetResult().GetValue().IsValid());
    EXPECT_SUCCESS(connection->QueryBriefcaseInfo(briefcaseA.GetValue()->GetId())->GetResult());
    }

TEST_F(BriefcaseTests, SuccessfulOpenBriefcase)
    {
    DgnDbPtr db = OpenBriefcaseFile();

    auto result = m_client->OpenBriefcase(db, false, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(result);

    EXPECT_TRUE(db->GetBriefcaseId().IsValid());
    EXPECT_FALSE(db->GetBriefcaseId().IsStandaloneId());
    EXPECT_FALSE(db->GetBriefcaseId().IsMasterId());
    CheckNoProgress();

    // We randomly were getting ASSERTION FAILURE (0): id==GetThreadId(). It was because the last db pointer was released during closing OpenBriefcase async task
    // and not from this test since test is short and it ends before async task ends. Async task starts closing only after we get result from it.
    result.GetValue() = nullptr;
    DeleteiModel(*m_client, *m_imodel);
    m_imodel = nullptr;
    m_client = nullptr;
    // It would be great to call StopThreadingAndWait to make sure async tasks are closed before this test ends.
    // Due to bug in HttpClient the second test fails, so commenting this out for now. Test do not fail any more since more work were added after OpenBriefcase.
    // AsyncTasksManager::StopThreadingAndWait();
    }

TEST_F(BriefcaseTests, UnauthorizedOpenBriefcase)
    {
    if (IntegrationTestSettings::Instance().IsIms())
        return;

    DgnDbPtr db = OpenBriefcaseFile();

    auto badClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetWrongPassword());

    auto result = badClient->OpenBriefcase(db, false, CreateProgressCallback())->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(Error::Id::LoginFailed, result.GetError().GetId());
    CheckNoProgress();

    DeleteiModel(*m_client, *m_imodel);
    m_imodel = nullptr;
    m_client = nullptr;
    // Same comment as in SuccessfulOpenBriefcase test.
    // AsyncTasksManager::StopThreadingAndWait();
    }

TEST_F(BriefcaseTests, SuccessfulAcquireBriefcaseWithoutMerge)
    {
    InitializeWithChangeSets();

    auto result = m_client->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory(), false, Client::DefaultFileNameCallback, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(result);

    auto dbPath = result.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());

    auto db = DgnDb::OpenDgnDb(nullptr, dbPath, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(db.IsValid());
    auto briefcaseResult = m_client->OpenBriefcase(db, false, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(briefcaseResult);
    CheckNoProgress();

    DeleteiModel(*m_client, *m_imodel);
    m_imodel = nullptr;
    m_client = nullptr;
    // Same comment as in SuccessfulOpenBriefcase test.
    // AsyncTasksManager::StopThreadingAndWait();
    }

TEST_F(BriefcaseTests, SuccessfulAcquireAndMergeBriefcase)
    {
    InitializeWithChangeSets();
    // TFS#735814 CreateProgressCallback()
    auto result = m_client->AcquireBriefcaseToDir(*m_imodel, m_pHost->GetOutputDirectory(), true)->GetResult();

    EXPECT_SUCCESS(result);
    auto dbPath = result.GetValue()->GetLocalPath();
    EXPECT_TRUE(dbPath.DoesPathExist());
    // TFS#735814 CheckProgressNotified();

    auto db = DgnDb::OpenDgnDb(nullptr, dbPath, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(db.IsValid());

    Utf8String lastChangeSetId = db->Revisions().GetParentRevisionId();
    EXPECT_FALSE(lastChangeSetId.empty());

    DeleteiModel(*m_client, *m_imodel);
    m_imodel = nullptr;
    m_client = nullptr;
    // Same comment as in SuccessfulOpenBriefcase test.
    // AsyncTasksManager::StopThreadingAndWait();
    }

TEST_F(BriefcaseTests, SuccessfulOpenAndMergeBriefcase)
    {
    InitializeWithChangeSets();
    DgnDbPtr db = OpenBriefcaseFile();

    // TFS#735814 CreateProgressCallback()
    auto result = m_client->OpenBriefcase(db, true)->GetResult();
    EXPECT_SUCCESS(result);
    // TFS#735814 CheckProgressNotified();

    Utf8String lastChangeSetId = db->Revisions().GetParentRevisionId();
    EXPECT_FALSE(lastChangeSetId.empty());
    }


TEST_F(BriefcaseTests, OpenSeedFileAsBriefcase)
    {
    DgnDbPtr db = CreateTestDb("BriefcaseTest");
    EXPECT_TRUE(db.IsValid());

    auto createResult = m_client->CreateNewiModel(*db, true, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(createResult);
    CheckProgressNotified();

    auto result = m_client->OpenBriefcase(db, false, CreateProgressCallback())->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(Error::Id::FileIsNotBriefcase, result.GetError().GetId());
    DeleteiModel(*m_client, *createResult.GetValue());
    CheckNoProgress();
    }

TEST_F(BriefcaseTests, PullAndMerge)
    {
    InitializeWithChangeSets();
    BriefcasePtr briefcase = AcquireBriefcase();
    DgnDbR db = briefcase->GetDgnDb();
    Utf8String lastChangeSetId;
 
    auto upToDateResult = briefcase->IsBriefcaseUpToDate()->GetResult();
    EXPECT_SUCCESS(upToDateResult);
    EXPECT_FALSE(upToDateResult.GetValue());
        {
        // TFS#735814 CreateProgressCallback()
        auto result = briefcase->PullAndMerge()->GetResult();
        EXPECT_SUCCESS(result);
        EXPECT_EQ(2, result.GetValue().size());
        // TFS#735814 CheckProgressNotified();

        lastChangeSetId = db.Revisions().GetParentRevisionId();
        EXPECT_FALSE(lastChangeSetId.empty());
        EXPECT_EQ(lastChangeSetId, result.GetValue().back()->GetId());
        }
    
    upToDateResult = briefcase->IsBriefcaseUpToDate()->GetResult();
    EXPECT_SUCCESS(upToDateResult);
    EXPECT_TRUE(upToDateResult.GetValue());     

    auto changeSetsResult = briefcase->GetiModelConnection().GetAllChangeSets()->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    EXPECT_EQ(2, changeSetsResult.GetValue().size());

    auto lastChangeSet = changeSetsResult.GetValue().back();
    EXPECT_EQ(lastChangeSetId, lastChangeSet->GetId());
    EXPECT_NE("", lastChangeSet->GetDbGuid());
    EXPECT_GT(lastChangeSet->GetIndex(), 0);
    EXPECT_EQ("", lastChangeSet->GetDescription());
    EXPECT_GT(lastChangeSet->GetFileSize(), 0);
    EXPECT_NE("", lastChangeSet->GetUserCreated());
    EXPECT_GE(lastChangeSet->GetPushDate().GetYear(), 2017);
    EXPECT_GT(lastChangeSet->GetBriefcaseId().GetValue(), 0u);
    EXPECT_EQ(0, lastChangeSet->GetContainingChanges());
    }

TEST_F(BriefcaseTests, Push)
    {
    auto briefcase = AcquireBriefcase();
    DgnDbR db = briefcase->GetDgnDb();

    auto model = CreateModel ("TestModel", db);
    CreateElement(*model, false);
    BeSQLite::DbResult saveResult = db.SaveChanges();
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, saveResult);

    auto result = briefcase->PullMergeAndPush(nullptr, false, nullptr, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(result);
    CheckProgressNotified();

    Utf8String lastChangeSetId = db.Revisions().GetParentRevisionId();
    EXPECT_FALSE(lastChangeSetId.empty());
    }

TEST_F(BriefcaseTests, PullMergeAndPush)
    {
    InitializeWithChangeSets();
    auto briefcase = AcquireBriefcase();
    DgnDbR db = briefcase->GetDgnDb();

    Utf8String lastChangeSetId = db.Revisions().GetParentRevisionId();
    
    auto model = CreateModel ("TestModel", db);
    CreateElement(*model, false);
    BeSQLite::DbResult saveResult = db.SaveChanges();
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, saveResult);

    auto result = briefcase->PullMergeAndPush(nullptr, false, nullptr, CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(result);
    CheckProgressNotified();

    lastChangeSetId = db.Revisions().GetParentRevisionId();
    EXPECT_FALSE(lastChangeSetId.empty());
    }

TEST_F(BriefcaseTests, TwoPulls)
    {
    InitializeWithChangeSets();
    auto briefcase = AcquireBriefcase();
    DgnDbR db = briefcase->GetDgnDb();

    Utf8String lastChangeSetId = db.Revisions().GetParentRevisionId();
    auto result = briefcase->PullAndMerge(CreateProgressCallback())->GetResult();
    EXPECT_SUCCESS(result);
    CheckProgressNotified();

    lastChangeSetId = db.Revisions().GetParentRevisionId();
    EXPECT_FALSE(lastChangeSetId.empty());

    InitializeWithChangeSets();
    // TFS#735814 CreateProgressCallback()
    result = briefcase->PullAndMerge()->GetResult();
    EXPECT_SUCCESS(result);
    // TFS#735814 CheckProgressNotified();

    Utf8String lastChangeSetId2 = db.Revisions().GetParentRevisionId();
    EXPECT_FALSE(lastChangeSetId2.empty());
    EXPECT_NE(lastChangeSetId2, lastChangeSetId);
    }

int GetDirSize(BeFileName dirToCheck)
    {
    BeFileListIterator fileIterator(dirToCheck.GetName(), false);
    BeFileName tempFileName;
    int size = 0;
    
    while(SUCCESS == fileIterator.GetNextFileName(tempFileName))
        {
        size++;
        }

    return size;
    }

bool IsDirEmpty(BeFileName dirToCheck)
    {
    return 0 == GetDirSize(dirToCheck);
    }

TEST_F(BriefcaseTests, PreDownload)
    {
    Configuration::SetPredownloadChangeSetsEnabled(true);
    BeFileName preDownloadPath;
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(preDownloadPath, L"DgnDbRev\\PreDownload");
    BeAssert(SUCCESS == status && "Cannot get pre-download directory");
    preDownloadPath.AppendToPath(L"*");
    EXPECT_TRUE(IsDirEmpty(preDownloadPath));

    auto briefcase = AcquireBriefcase();
    InitializeWithChangeSets();

    // Wait max 50 sec until changeSet files are preDownloaded
    int maxIterations = 100;
    while (IsDirEmpty(preDownloadPath) && maxIterations > 0)
        {
        BeThreadUtilities::BeSleep(500);
        maxIterations--;
        }
    EXPECT_FALSE(IsDirEmpty(preDownloadPath));

    // Check if PullAndMerge removes preDownloaded files
    auto result = briefcase->PullAndMerge()->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_TRUE(IsDirEmpty(preDownloadPath));
    Configuration::SetPredownloadChangeSetsEnabled(false);
    }

TEST_F(BriefcaseTests, PreDownloadSmallCacheSize)
    {
    Configuration::SetPredownloadChangeSetsEnabled(true);
    Configuration::SetPredownloadChangeSetsCacheSize(1);

    BeFileName preDownloadPath;
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(preDownloadPath, L"DgnDbRev\\PreDownload");
    BeAssert(SUCCESS == status && "Cannot get pre-download directory");
    preDownloadPath.AppendToPath(L"*");
    EXPECT_TRUE(IsDirEmpty(preDownloadPath));

    auto briefcase = AcquireBriefcase();
    InitializeWithChangeSets();

    // Wait max 20 sec until changeSet files are preDownloaded
    int maxIterations = 40;
    while (maxIterations > 0)
        {
        if (1 == GetDirSize(preDownloadPath))
            break;

        BeThreadUtilities::BeSleep(500);
        maxIterations--;
        }

    EXPECT_EQ(1, GetDirSize(preDownloadPath));
    Configuration::SetPredownloadChangeSetsCacheSize(10 * 1024 * 1024);
    Configuration::SetPredownloadChangeSetsEnabled(false);
    }

TEST_F(BriefcaseTests, PreDownloadTurnedOff)
    {
    BeFileName preDownloadPath;
    BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(preDownloadPath, L"DgnDbRev\\PreDownload");
    BeAssert(SUCCESS == status && "Cannot get pre-download directory");
    preDownloadPath.AppendToPath(L"*");
    EXPECT_TRUE(IsDirEmpty(preDownloadPath));

    auto briefcase = AcquireBriefcase();
    InitializeWithChangeSets();

    BeThreadUtilities::BeSleep(1000);
    EXPECT_TRUE(IsDirEmpty(preDownloadPath));
    }

TEST_F(BriefcaseTests, PreDownloadManyBriefcases)
    {
    Configuration::SetPredownloadChangeSetsEnabled(true);
    // Acquire 5 briefcases
    bvector<BriefcasePtr> briefcases;
    for (int i = 0; i < 4; i++)
        {
        briefcases.push_back(AcquireBriefcase());
        }

    // Initialize some changeSets
    InitializeWithChangeSets();

    // Pull all briefcases & check all have the same changeSet
    Utf8String lastChangeSet;
    for (auto it = briefcases.begin(); it != briefcases.end(); ++it)
        {
        auto currentBriefcase = *it;
        EXPECT_TRUE(currentBriefcase->PullAndMerge()->GetResult().IsSuccess());

        if (Utf8String::IsNullOrEmpty(lastChangeSet.c_str()))
            lastChangeSet = currentBriefcase->GetLastChangeSetPulled();
        else
            EXPECT_EQ(lastChangeSet, currentBriefcase->GetLastChangeSetPulled());
        Configuration::SetPredownloadChangeSetsEnabled(false);
        }
    }

TEST_F(BriefcaseTests, DownloadLocalBriefcaseUpdatedToVersion)
    {
    //create changeSets
    IntegrationTestsBase::InitializeWithChangeSets(*m_client, *m_imodel, 7);
    auto changeSetsResult = m_imodelConnection->GetAllChangeSets()->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    auto changeSets = changeSetsResult.GetValue();

    auto versionManager = m_imodelConnection->GetVersionsManager();
    VersionInfoPtr version1 = new VersionInfo("Version1", "Description", changeSets.at(2)->GetId());
    auto versionResult = versionManager.CreateVersion(*version1)->GetResult();
    EXPECT_SUCCESS(versionResult);

    VersionInfoPtr version2 = new VersionInfo("Version2", "Description", changeSets.at(4)->GetId());
    versionResult = versionManager.CreateVersion(*version2)->GetResult();
    EXPECT_SUCCESS(versionResult);
    version2 = versionResult.GetValue();

    VersionInfoPtr version3 = new VersionInfo("Version3", "Description", changeSets.at(6)->GetId());
    versionResult = versionManager.CreateVersion(*version3)->GetResult();
    EXPECT_SUCCESS(versionResult);

    auto acquireResult = m_client->DownloadLocalBriefcaseUpdatedToVersion(*m_imodel, version2->GetId(), [=] (iModelInfo imodelInfo, FileInfo fileInfo)
        {
        BeFileName filePath = m_pHost->GetOutputDirectory();
        filePath.AppendToPath(BeFileName(imodelInfo.GetId()));
        filePath.AppendToPath(BeFileName(version2->GetId()));
        filePath.AppendToPath(BeFileName(fileInfo.GetFileName()));
        return filePath;
        })->GetResult();
    EXPECT_SUCCESS(acquireResult);

    BeFileName dbPath = acquireResult.GetValue();
    EXPECT_TRUE(dbPath.DoesPathExist());
    EXPECT_TRUE(dbPath.IsFileReadOnly());

    DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, dbPath, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(db.IsValid());

    auto briefcaseResult = m_client->OpenBriefcase(db, false)->GetResult();
    EXPECT_EQ(Error::Id::FileIsNotBriefcase, briefcaseResult.GetError().GetId());
    EXPECT_EQ(version2->GetChangeSetId(), db->Revisions().GetParentRevisionId());

    PhysicalPartitionPtr partition = CreateModeledElement("TestModel", *db);
    EXPECT_FALSE(partition->Insert().IsValid());
    }

TEST_F(BriefcaseTests, DownloadLocalBriefcaseUpdatedToChangeSet)
    {
    //create changeSets
    IntegrationTestsBase::InitializeWithChangeSets(*m_client, *m_imodel, 4);
    auto changeSetsResult = m_imodelConnection->GetAllChangeSets()->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    auto changeSets = changeSetsResult.GetValue();

    auto briefcaseChangeSet = changeSets.at(1)->GetId();
    auto acquireResult = m_client->DownloadLocalBriefcaseUpdatedToChangeSet(*m_imodel, briefcaseChangeSet, [=] (iModelInfo imodelInfo, FileInfo fileInfo)
        {
        BeFileName filePath = m_pHost->GetOutputDirectory();
        filePath.AppendToPath(BeFileName(imodelInfo.GetId()));
        filePath.AppendToPath(BeFileName(briefcaseChangeSet));
        filePath.AppendToPath(BeFileName(fileInfo.GetFileName()));
        return filePath;
        })->GetResult();
    EXPECT_SUCCESS(acquireResult);

    auto dbPath = acquireResult.GetValue();
    EXPECT_TRUE(dbPath.DoesPathExist());
    EXPECT_TRUE(dbPath.IsFileReadOnly());

    auto db = DgnDb::OpenDgnDb(nullptr, dbPath, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(db.IsValid());

    auto briefcaseResult = m_client->OpenBriefcase(db, false)->GetResult();
    EXPECT_EQ(Error::Id::FileIsNotBriefcase, briefcaseResult.GetError().GetId());
    EXPECT_EQ(briefcaseChangeSet, db->Revisions().GetParentRevisionId());

    PhysicalPartitionPtr partition = CreateModeledElement("TestModel", *db);
    EXPECT_FALSE(partition->Insert().IsValid());
    }

TEST_F(BriefcaseTests, DownloadLocalBriefcase)
    {
    //create changeSets
    IntegrationTestsBase::InitializeWithChangeSets(*m_client, *m_imodel, 4);
    auto changeSetsResult = m_imodelConnection->GetAllChangeSets()->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    auto changeSets = changeSetsResult.GetValue();

    auto acquireResult = m_client->DownloadLocalBriefcase(*m_imodel, [=] (iModelInfo imodelInfo, FileInfo fileInfo)
        {
        BeFileName filePath = m_pHost->GetOutputDirectory();
        filePath.AppendToPath(BeFileName(imodelInfo.GetId()));
        filePath.AppendToPath(BeFileName(fileInfo.GetFileName()));
        return filePath;
        })->GetResult();
    EXPECT_SUCCESS(acquireResult);

    auto dbPath = acquireResult.GetValue();
    EXPECT_TRUE(dbPath.DoesPathExist());
    EXPECT_TRUE(dbPath.IsFileReadOnly());

    auto db = DgnDb::OpenDgnDb(nullptr, dbPath, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(db.IsValid());

    auto briefcaseResult = m_client->OpenBriefcase(db, false)->GetResult();
    EXPECT_EQ(Error::Id::FileIsNotBriefcase, briefcaseResult.GetError().GetId());
    EXPECT_EQ(changeSets.at(3)->GetId(), db->Revisions().GetParentRevisionId());

    PhysicalPartitionPtr partition = CreateModeledElement("TestModel", *db);
    EXPECT_FALSE(partition->Insert().IsValid());
    }