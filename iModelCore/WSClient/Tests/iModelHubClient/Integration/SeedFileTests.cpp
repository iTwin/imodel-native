/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct SeedFileTests : public IntegrationTestsBase
    {
    static DgnDbPtr             s_db;
    static iModelConnectionPtr  s_connection;
    static const Utf8String     s_iModelName;
    static iModelInfoPtr        s_info;
    BriefcasePtr                m_briefcase;

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        IntegrationTestsBase::SetUpTestCase();
        ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, s_iModelName));
        s_db = CreateTestDb(s_iModelName);
        iModelResult result = CreateiModel(s_db);
        ASSERT_SUCCESS(result);
        s_info = result.GetValue();
        s_connection = CreateiModelConnection(s_info);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        if (s_db.IsValid())
            s_db = nullptr;
        if (s_connection.IsValid())
            s_connection = nullptr;
        iModelHubHelpers::DeleteiModelByName(s_client, s_iModelName);
        IntegrationTestsBase::TearDownTestCase();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetUp() override
        {
        IntegrationTestsBase::SetUp();
        ASSERT_SUCCESS(iModelHubHelpers::AbandonAllBriefcases(s_client, s_connection));
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void TearDown() override
        {
        iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());
        if (m_briefcase.IsValid())
            m_briefcase = nullptr;
        s_connection->UnlockiModel()->GetResult();
        IntegrationTestsBase::TearDown();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static FileInfoPtr CreateFileInfo()
        {
        return FileInfo::Create(*s_db, ::testing::UnitTest::GetInstance()->current_test_info()->name());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void GetLatestFile(FileResult& result)
        {
        result = s_connection->GetLatestSeedFile()->GetResult();
        ASSERT_SUCCESS(result);
        }
    
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BeGuid GetLatestFileId()
        {
        FileResult result;
        GetLatestFile(result);
        return result.GetValue()->GetFileId();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void ExpectLatestFileGuid(BeGuid guid)
        {
        FileResult result;
        GetLatestFile(result);
        EXPECT_EQ(guid, result.GetValue()->GetFileId());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BeGuid ReplaceSeedFile()
        {
        BeGuid guid = iModelHubHelpers::ReplaceSeedFile(s_connection, s_db);
        ExpectLatestFileGuid(guid);
        return guid;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AcquireBriefcase()
        {
        BriefcaseResult acquireResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, s_info);
        ASSERT_SUCCESS(acquireResult);
        m_briefcase = acquireResult.GetValue();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    DgnDbPtr RecoverBriefcase(ClientPtr client, DgnDbR db)
        {
        auto briefcaseName = db.GetFileName();
        db.CloseDb();

        DgnDbPtr db2 = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        TestsProgressCallback callback;
        StatusResult recoverResult = client->RecoverBriefcase(db2, callback.Get())->GetResult();
        EXPECT_SUCCESS(recoverResult);
        if (!recoverResult.IsSuccess())
            return nullptr;
        callback.Verify();

        DgnDbPtr resultDb = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        return resultDb;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void RecoverAndOpenBriefcase()
        {
        DgnDbPtr db = RecoverBriefcase(s_client, m_briefcase->GetDgnDb());
        ASSERT_TRUE(db.IsValid());
        m_briefcase = nullptr;

        // Open briefcase should now succeed
        BriefcaseResult result = iModelHubHelpers::OpenBriefcase(s_client, db);
        ASSERT_SUCCESS(result);
        m_briefcase = result.GetValue();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void GetSeedFiles(FilesResult& result)
        {
        result = s_connection->GetSeedFiles()->GetResult();
        ASSERT_SUCCESS(result);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static size_t GetSeedFilesCount()
        {
        FilesResult result;
        GetSeedFiles(result);
        return result.GetValue().size();
        }
    };

DgnDbPtr SeedFileTests::s_db = nullptr;
iModelConnectionPtr SeedFileTests::s_connection = nullptr;
iModelInfoPtr SeedFileTests::s_info = nullptr;
const Utf8String SeedFileTests::s_iModelName = "SeedFileTests";


/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SeedFileTests, ReplaceWithoutLockediModel)
    {
    FileResult result;
    iModelHubHelpers::UploadNewSeedFile(result, s_connection, s_db, false);
    EXPECT_NE(Error::Id::Unknown, result.GetError().GetId()) << "TFS#804291";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SeedFileTests, ReplaceWithLockediModel)
    {
    size_t count = GetSeedFilesCount();
    ReplaceSeedFile();
    EXPECT_LE(count + 1, GetSeedFilesCount());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Algirdas.Mikoliunas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SeedFileTests, DownloadArchivedSeedFile)
    {
    BeGuid originalGuid = GetLatestFileId();

    iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 2);
    
    ReplaceSeedFile();

    auto queryResult = s_connection->GetSeedFileById(originalGuid)->GetResult();
    ASSERT_SUCCESS(queryResult);

    // Download original seed file
    BeFileName originalFilePath = iModelHubHost::Instance().BuildDbFileName("OriginalFile");
    TestsProgressCallback callback;
    auto downloadResult = s_connection->DownloadSeedFile(originalFilePath, queryResult.GetValue()->GetFileId().ToString(), callback.Get())->GetResult();
    ASSERT_SUCCESS(downloadResult);
    callback.Verify();

    // Download available changeSets
    TestsProgressCallback callback2;
    auto changeSetsResult = s_connection->DownloadChangeSetsAfterId("", originalGuid, callback2.Get())->GetResult();
    ASSERT_SUCCESS(changeSetsResult);
    callback2.Verify();
    auto changeSets = changeSetsResult.GetValue();
    EXPECT_EQ(2, changeSets.size());

    // Open original seed file
    DgnDbPtr originalDb = DgnDb::OpenDgnDb(nullptr, originalFilePath, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    ASSERT_TRUE(originalDb.IsValid());

    // Attempt to merge the changeSets
    originalDb->SetAsBriefcase(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
    originalDb->ChangeDbGuid(originalGuid);
    originalDb->SaveChanges();
    originalDb->CloseDb();
    originalDb = DgnDb::OpenDgnDb(nullptr, originalFilePath, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    for (auto const& changeSet : changeSets)
        {
        RevisionStatus status = originalDb->Revisions().MergeRevision(*changeSet);
        EXPECT_EQ(RevisionStatus::Success, status);
        }
    originalDb->CloseDb();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SeedFileTests, PushAndPullAfterReplacement)
    {
    ReplaceSeedFile();
    iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info);
    iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SeedFileTests, QueryNonExistentSeedFile)
    {
    FileResult result = s_connection->GetSeedFileById(BeGuid(true))->GetResult();
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::FileDoesNotExist, result.GetError().GetId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            08/2016
//---------------------------------------------------------------------------------------
TEST_F(SeedFileTests, ReplaceSeedFileWithLocks)
    {
    // Create imodel and acquire a briefcase
    BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, s_info, true);
    ASSERT_SUCCESS(briefcaseResult);
    BriefcasePtr briefcase = briefcaseResult.GetValue();

    // Acquire a lock
    AcquireLock(briefcase->GetDgnDb(), LockLevel::Exclusive);

    // Replace seed file should fail
    ASSERT_SUCCESS(s_connection->LockiModel()->GetResult());
    FileResult result;
    iModelHubHelpers::UploadNewSeedFile(result, s_connection, s_db, false);
    ASSERT_SUCCESS(s_connection->UnlockiModel()->GetResult());

    // Release the db lock
    ASSERT_EQ(RepositoryStatus::Success, briefcase->GetDgnDb().BriefcaseManager().RelinquishLocks());

    // Replace seed file should succeed now
    ReplaceSeedFile();
    }

void CreateFileInstance(iModelConnectionPtr connection)
    {
    Json::Value createFileJson = Json::objectValue;
    createFileJson["instance"] = Json::objectValue;
    createFileJson["instance"]["schemaName"] = "iModelScope";
    createFileJson["instance"]["className"] = "SeedFile";
    createFileJson["instance"]["properties"]["FileId"] = BeSQLite::BeGuid(true).ToString();
    createFileJson["instance"]["properties"]["MergedChangeSetId"] = "";
    auto wsRepositoryClient = connection->GetRepositoryClient();
    auto requestOptions = iModelHubHelpers::CreateiModelHubRequestOptions();
    ASSERT_SUCCESS(wsRepositoryClient->SendCreateObjectRequestWithOptions(createFileJson, BeFileName(), nullptr, requestOptions)->GetResult());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            08/2016
//---------------------------------------------------------------------------------------
TEST_F(SeedFileTests, DeleteFile)
    {
    EXPECT_FAILURE(s_connection->CancelSeedFileCreation()->GetResult());
    size_t size = GetSeedFilesCount();
    EXPECT_LE(1, size);

    ASSERT_SUCCESS(s_connection->LockiModel()->GetResult());
    CreateFileInstance(s_connection);
    EXPECT_EQ(size + 1, GetSeedFilesCount());

    ASSERT_SUCCESS(s_connection->CancelSeedFileCreation()->GetResult());
    ASSERT_SUCCESS(s_connection->UnlockiModel()->GetResult());
    EXPECT_EQ(size, GetSeedFilesCount());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            08/2016
//---------------------------------------------------------------------------------------
TEST_F(SeedFileTests, DeleteFileWhenNotReplacing)
    {
    StatusResult result = s_connection->CancelSeedFileCreation()->GetResult();
    EXPECT_FAILURE(result);
    EXPECT_EQ(Error::Id::FileDoesNotExist, result.GetError().GetId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            08/2016
//---------------------------------------------------------------------------------------
TEST_F(SeedFileTests, OpenBriefcaseAfterReplace)
    {
    BeGuid oldGuid = GetLatestFileId();
    AcquireBriefcase();
    ReplaceSeedFile();

    // Close briefcase
    DgnDbR briefcaseDb = m_briefcase->GetDgnDb();
    auto briefcaseName = briefcaseDb.GetFileName();
    briefcaseDb.CloseDb();
    m_briefcase = nullptr;

    // Open briefcase should fail
    DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    BriefcaseResult result = iModelHubHelpers::OpenBriefcase(s_client, db, false, false);
    ASSERT_FAILURE(result);
    EXPECT_EQ(Error::Id::InvalidBriefcase, result.GetError().GetId());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            08/2016
//---------------------------------------------------------------------------------------
TEST_F(SeedFileTests, RecoverBriefcaseAfterReplace)
    {
    BeGuid oldGuid = GetLatestFileId();
    AcquireBriefcase();
    BeGuid newGuid = ReplaceSeedFile();
    RecoverAndOpenBriefcase();

    // Open briefcase should now succeed
    EXPECT_EQ(newGuid, m_briefcase->GetDgnDb().GetDbGuid());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas            09/2016
//---------------------------------------------------------------------------------------
TEST_F(SeedFileTests, CanceledFileReplaceCodesLocksChangeSets)
    {
    AcquireBriefcase();
    DgnDbR db = m_briefcase->GetDgnDb();
    IRepositoryManagerP manager = _GetRepositoryManager(db);

    // Start and cancel seed file replace
    ASSERT_SUCCESS(s_connection->LockiModel()->GetResult());
    CreateFileInstance(s_connection);
    ASSERT_SUCCESS(s_connection->CancelSeedFileCreation()->GetResult());
    ASSERT_SUCCESS(s_connection->UnlockiModel()->GetResult());

    // Create code
    DgnCode code = MakeStyleCode(TestCodeName(1).c_str(), db);
    iModelHubHelpers::ExpectCodesCount(m_briefcase, 0);

    // Reserve code
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(code));;
    iModelHubHelpers::ExpectCodesCount(m_briefcase, 1);
    ExpectCodeState(CreateCodeReserved(code, db), manager);

    // Use code + lock
    auto model1 = CreateModel(TestCodeName(2).c_str(), db);
    InsertStyle(TestCodeName(1).c_str(), db);
    db.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(m_briefcase, true, false, false));

     iModelHubHelpers::ExpectLocksCount(m_briefcase, 6);
    iModelHubHelpers::ExpectCodesCount(m_briefcase, 0);
    ExpectCodeState(CreateCodeUsed(code, db.Revisions().GetParentRevisionId()), manager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas            09/2016
//---------------------------------------------------------------------------------------
TEST_F(SeedFileTests, FileReplaceInProgressCodesLocksChangeSetsDenied)
    {
    // Create imodel and acquire a briefcase
    AcquireBriefcase();
    DgnDbR db = m_briefcase->GetDgnDb();
    IRepositoryManagerP manager = _GetRepositoryManager(db);

    // Start seed file replace
    EXPECT_SUCCESS(s_connection->LockiModel()->GetResult());

    // Create code
    DgnCode code = MakeStyleCode("Code1", db);
    iModelHubHelpers::ExpectCodesCount(m_briefcase, 0);

    // Reserve code
    RepositoryStatus reserveCodeResult = db.BriefcaseManager().ReserveCode(code);
    EXPECT_EQ(RepositoryStatus::RepositoryIsLocked, reserveCodeResult);
    iModelHubHelpers::ExpectCodesCount(m_briefcase, 0);
    ExpectNoCodeWithState(CreateCodeReserved(code, db), manager);

    // Check create model fails
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), TestCodeName(1).c_str());
    EXPECT_TRUE(partition.IsValid());
    auto stat = db.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::RepositoryIsLocked, stat);

    PhysicalModelPtr model = PhysicalModel::Create(*partition);
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::RepositoryIsLocked, db.BriefcaseManager().PrepareForModelInsert(req, *model, IBriefcaseManager::PrepareAction::Acquire));

    db.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(m_briefcase, false, false, false));

    // Check nothing changed
    iModelHubHelpers::ExpectLocksCount(m_briefcase, 0);
    iModelHubHelpers::ExpectCodesCount(m_briefcase, 0);
    ExpectNoCodeWithState(CreateCodeUsed(partition->GetCode(), db.Revisions().GetParentRevisionId()), manager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas            09/2016
//---------------------------------------------------------------------------------------
TEST_F(SeedFileTests, ReplaceCodesLocksChangeSets)
    {
    AcquireBriefcase();
    ReplaceSeedFile();
    RecoverAndOpenBriefcase();
    DgnDbR db = m_briefcase->GetDgnDb();
    auto manager = _GetRepositoryManager(db);

    // Create code
    DgnCode code = MakeStyleCode(TestCodeName(1).c_str(), db);
    DgnCode code2 = MakeStyleCode(TestCodeName(2).c_str(), db);
    iModelHubHelpers::ExpectCodesCount(m_briefcase, 0);

    // Reserve code
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(code));
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(code2));
    iModelHubHelpers::ExpectCodesCount(m_briefcase, 2);
    ExpectCodeState(CreateCodeReserved(code, db), manager);

    // Use code + lock
    auto model1 = CreateModel(TestCodeName(3).c_str(), db);
    InsertStyle(TestCodeName(1).c_str(), db);
    db.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(m_briefcase, true, false, false));

    iModelHubHelpers::ExpectLocksCount(m_briefcase, 6);
    iModelHubHelpers::ExpectCodesCount(m_briefcase, 1);
    ExpectCodeState(CreateCodeUsed(code, db.Revisions().GetParentRevisionId()), manager);
    ExpectCodeState(CreateCodeReserved(code2, db), manager);

    // Discard one code and use another
    auto pStyle = AnnotationTextStyle::GetForEdit(db.GetDictionaryModel(), TestCodeName(1).c_str());
    pStyle->SetName(TestCodeName(2).c_str());
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    // Push changes
    db.SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(m_briefcase, true, false, false));

    ExpectCodeState(CreateCodeUsed(code2, db.Revisions().GetParentRevisionId()), manager);
    ExpectNoCodeWithState(CreateCodeDiscarded(code, db.Revisions().GetParentRevisionId()), manager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas            09/2016
//---------------------------------------------------------------------------------------
TEST_F(SeedFileTests, EmptyFileReplaceOtherUser)
    {
    AcquireBriefcase();
    ClientPtr client = CreateNonAdminClient();

    // Create new user
    BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(client, s_info);
    ASSERT_SUCCESS(briefcaseResult);
    BriefcasePtr briefcase2 = briefcaseResult.GetValue();
    DgnDbR db2 = briefcase2->GetDgnDb();

    iModelHubHost::Instance().SetRepositoryAdmin(client->GetiModelAdmin());

    // Do something to initialize briefcaseManager
    EXPECT_EQ(RepositoryStatus::Success, db2.BriefcaseManager().ReserveCode(MakeStyleCode(TestCodeName().c_str(), db2)));
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle(TestCodeName(1).c_str(), db2));
    db2.SaveChanges();

    auto result = briefcase2->GetiModelConnection().RelinquishCodesLocks(briefcase2->GetBriefcaseId())->GetResult();
    EXPECT_SUCCESS(result);

    iModelHubHost::Instance().SetRepositoryAdmin(s_client->GetiModelAdmin());

    ReplaceSeedFile();

    // Refresh briefcase
    RecoverAndOpenBriefcase();
    iModelHubHost::Instance().SetRepositoryAdmin(client->GetiModelAdmin());

    // Create & reserve code should succeed
    DgnCode code2 = MakeStyleCode(TestCodeName(2).c_str(), db2);
    RepositoryStatus status = db2.BriefcaseManager().ReserveCode(code2);
    EXPECT_EQ(RepositoryStatus::Success, status);

    // Element creation should fail
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db2.Elements().GetRootSubject(), "PartitionName");
    EXPECT_TRUE(partition.IsValid());
    status = db2.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::RevisionRequired, status);

    // Locks should fail
    LockRequest lockReq;
    lockReq.Insert(m_briefcase->GetDgnDb(), LockLevel::Exclusive);
    status = db2.BriefcaseManager().AcquireLocks(lockReq).Result();
    EXPECT_EQ(RepositoryStatus::RevisionRequired, status);

    // ChangeSet push should fail
    ChangeSetsResult pushResult = briefcase2->PullMergeAndPush(nullptr, false)->GetResult();
    Error::Id errorId = pushResult.GetError().GetId();
    EXPECT_EQ(Error::Id::ChangeSetPointsToBadSeed, errorId);

    //Recover briefcase
    auto briefcase2Name = db2.GetFileName();
    partition = nullptr;
    DgnDbPtr newDb = RecoverBriefcase(client, db2);

    // Open new briefcase
    briefcaseResult = iModelHubHelpers::OpenBriefcase(client, newDb);
    ASSERT_SUCCESS(briefcaseResult);
    BriefcasePtr newBriefcase = briefcaseResult.GetValue();
    auto newiModelManager = _GetRepositoryManager(*newDb);

    // Create code
    DgnCode code = MakeStyleCode(TestCodeName(3).c_str(), *newDb);
    iModelHubHelpers::ExpectCodesCount(newBriefcase, 1);

    // Reserve code
    EXPECT_EQ(RepositoryStatus::Success, newDb->BriefcaseManager().ReserveCode(code));
    iModelHubHelpers::ExpectCodesCount(newBriefcase, 2);
    ExpectCodeState(CreateCodeReserved(code, *newDb), newiModelManager);

    // Create and push model
    auto model1 = CreateModel(TestCodeName(4).c_str(), *newDb);
    newDb->SaveChanges();
    ASSERT_SUCCESS(iModelHubHelpers::PullMergeAndPush(newBriefcase, true, false, false));

    iModelHubHelpers::ExpectLocksCount(newBriefcase, 4);
    iModelHubHelpers::ExpectCodesCount(newBriefcase, 2);
    }
