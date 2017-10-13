/*--------------------------------------------------------------------------------------+
|
|  $Source: iModelBridge/Tests/NonPublished/Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/SHA1.h>
#include <iModelBridge/iModelBridge.h>
#include <iModelBridge/iModelBridgeSyncInfoFile.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>
#include <iModelBridge/iModelBridgeFwk.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <DgnPlatform/GenericDomain.h>
#include "../../Fwk/DgnDbServerClientUtils.h"
#include <Bentley/BeFileName.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// @bsistruct                                                   Sam.Wilson   04/17
//=======================================================================================
struct iModelBridgeTests : ::testing::Test
{
    static BeFileName GetOutputDir()
        {
        BeFileName bcDir;
        BeTest::GetHost().GetOutputRoot(bcDir);
        bcDir.AppendToPath(L"iModelBridgeTests");
        return bcDir;
        }

    static BeFileName GetSeedFilePath() {auto path=GetOutputDir(); path.AppendToPath(L"seed.bim"); return path;}

    static void SetUpTestCase()
        {
        ScopedDgnHost host;
        BeFileName seedDbName(GetSeedFilePath());
        BeFileName::CreateNewDirectory(seedDbName.GetDirectoryName().c_str());

        // Initialize parameters needed to create a DgnDb
        CreateDgnDbParams createProjectParams;
        createProjectParams.SetRootSubjectName("iModelBridgeTests");

        // Create the seed DgnDb file. The BisCore domain schema is also imported. 
        BeSQLite::DbResult createStatus;
        DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, seedDbName, createProjectParams);
        ASSERT_TRUE(db.IsValid());

        // Set up a model and category to use
        DgnDbTestUtils::InsertPhysicalModel(*db, "PhysicalModel");
        DgnDbTestUtils::InsertSpatialCategory(*db, "SpatialCategory");

        // Force the seed db to have non-zero briefcaseid, so that changes made to it will be in a txn
        db->SetAsBriefcase(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
        db->SaveChanges();
        }

    static DgnModelId GetPhysicalModel(DgnDbR db)
        {
        auto partitionCode = PhysicalPartition::CreateCode(*db.Elements().GetRootSubject(), "PhysicalModel");
        auto partitionId = db.Elements().QueryElementIdByCode(partitionCode);
        auto partition = db.Elements().Get<PhysicalPartition>(partitionId);
        return partition.IsValid()? partition->GetSubModelId(): DgnModelId();
        }

    static DgnCategoryId GetSpatialCategory(DgnDbR db)
        {
        return SpatialCategory::QueryCategoryId(db.GetDictionaryModel(), "SpatialCategory");
        }

    static DgnClassId GetGenericPhysicalObjectClassId(DgnDbR db)
        {
        return db.Domains().GetClassId(generic_ElementHandler::PhysicalObject::GetHandler());
        }

    static GenericPhysicalObjectPtr CreateGenericPhysicalObject(DgnDbR db)
        {
        DgnModelId modelId = GetPhysicalModel(db);
        DgnCategoryId categoryId = GetSpatialCategory(db);
        DgnClassId classId = GetGenericPhysicalObjectClassId(db);
        return new GenericPhysicalObject(GenericPhysicalObject::CreateParams(db, modelId, classId, categoryId));
        }

    static void GetWriteableCopyOfSeed(BeFileNameR cpPath, WCharCP cpName)
        {
        BeFileName seedDbName = GetSeedFilePath();
        cpPath = BeFileName(seedDbName.GetDirectoryName());
        cpPath.AppendToPath(cpName);
        BeFileName::CreateNewDirectory(cpPath.GetDirectoryName().c_str());
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seedDbName, cpPath));
        }
};

//=======================================================================================
// @bsistruct                                                   Sam.Wilson   04/17
//=======================================================================================
struct iModelBridgeSyncInfoFileTester : iModelBridgeBase
{
    WString _SupplySqlangRelPath() override {return L"sqlang/DgnPlatform_en.sqlang.db3";}
    BentleyStatus _Initialize(int argc, WCharCP argv[]) override {return BSISUCCESS;}
    BentleyStatus _ConvertToBim(SubjectCR jobSubject) override {DoTests(jobSubject); return BentleyStatus::SUCCESS;}
    SubjectCPtr _FindJob() override {return nullptr;}
    SubjectCPtr _InitializeJob() override {return nullptr;}
    void _DeleteSyncInfo() override {iModelBridgeSyncInfoFile::DeleteSyncInfoFileFor(_GetParams().GetBriefcaseName());}
    void _OnSourceFileDeleted() override {}

    void DoTests(SubjectCR jobSubject);

    iModelBridgeSyncInfoFileTester() : iModelBridgeBase() {}
};

//=======================================================================================
// @bsistruct                                                   Sam.Wilson   04/17
//=======================================================================================
struct TestSourceItemNoId : iModelBridgeSyncInfoFile::ISourceItem
    {
    Utf8String m_content;

    TestSourceItemNoId(Utf8StringCR content) : m_content(content) {}
    Utf8String _GetId() override  {return "";}
    double _GetLastModifiedTime() override {return 0.0;}
    Utf8String _GetHash() override
        {
        SHA1 sha1;
        sha1(m_content);
        return sha1.GetHashString();
        }
    };

//=======================================================================================
// @bsistruct                                                   Sam.Wilson   04/17
//=======================================================================================
struct TestSourceItemWithId : iModelBridgeSyncInfoFile::ISourceItem
    {
    Utf8String m_id;
    Utf8String m_content;

    TestSourceItemWithId(Utf8StringCR id, Utf8StringCR content) : m_id(id), m_content(content) {}
    Utf8String _GetId() override  {return m_id;}
    double _GetLastModifiedTime() override {return 0.0;}
    Utf8String _GetHash() override
        {
        SHA1 sha1;
        sha1(m_content);
        return sha1.GetHashString();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
static int32_t countElementsOfClass(DgnClassId classId, DgnDbR db)
    {
    CachedStatementPtr stmt = db.Elements().GetStatement("SELECT count(*) FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ECClassId=?");
    stmt->BindUInt64(1, classId.GetValue());
    stmt->Step();
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bpair<size_t,size_t> countItemsInSyncInfo(iModelBridgeSyncInfoFile& syncInfo)
    {
    size_t count = 0;
    size_t countThoseWithIds = 0;
    for (auto i : syncInfo.MakeIterator())
        {
        ++count;
        if (!i.GetSourceIdentity().GetId().empty())
            ++countThoseWithIds;
        }
    return make_bpair(count, countThoseWithIds);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void iModelBridgeSyncInfoFileTester::DoTests(SubjectCR jobSubject)
    {
    DgnDbR db = *m_db;
    iModelBridgeSyncInfoFile syncInfo;
    ASSERT_EQ(BentleyStatus::SUCCESS, syncInfo.AttachToBIM(db));

    // Items in scope1
    iModelBridgeSyncInfoFile::ROWID scope1;
    TestSourceItemNoId i0NoId("i0NoId initial");
    TestSourceItemNoId i1NoId("i1NoId initial");
    // Items in scope2
    iModelBridgeSyncInfoFile::ROWID scope2;
    TestSourceItemWithId i0WithId("0", "i0WithId initial");
    TestSourceItemWithId i1WithId("1", "i1WithId initial");


    Utf8CP itemKind = "";

    if (true)
        {
        iModelBridgeSyncInfoFile::ChangeDetector nullChangeDetector(syncInfo);
        // put the two scope items into syncinfo to start with.
        // Note that I don't have to create elements in order to put records into syncinfo. These records will
        // just serve as "scopes" to partition the items that I will "convert" below.
        iModelBridgeSyncInfoFile::SourceState noState(0.0, "");
        iModelBridgeSyncInfoFile::ConversionResults noElem;
        syncInfo.WriteResults(iModelBridgeSyncInfoFile::ROWID(), noElem, iModelBridgeSyncInfoFile::SourceIdentity(0, "Scope", "1"), noState, nullChangeDetector);
        scope1 = noElem.m_syncInfoRecord.GetROWID();
        noElem = iModelBridgeSyncInfoFile::ConversionResults(); // clear the previous results
        syncInfo.WriteResults(iModelBridgeSyncInfoFile::ROWID(), noElem, iModelBridgeSyncInfoFile::SourceIdentity(0, "Scope", "2"), noState, nullChangeDetector);
        scope2 = noElem.m_syncInfoRecord.GetROWID();
        }


    auto expected_counts = make_bpair<size_t,size_t>(2,2);  // (total count, count of items with IDs)

    if (true)
        {
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(syncInfo);

        // verify that the item does not exist in the BIM or in syncinfo
        iModelBridgeSyncInfoFile::ChangeDetector::Results change = changeDetector._DetectChange(scope1, itemKind, i0NoId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New, change.GetChangeType());

        // verify that second check on the same source item shows the same thing
        change = changeDetector._DetectChange(scope1, itemKind, i0NoId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New, change.GetChangeType());

        ASSERT_EQ(expected_counts, countItemsInSyncInfo(syncInfo));
        ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));

        // Do the same thing, only with a direct query on syncinfo
        if (true)
            {
            auto elems = syncInfo.MakeIteratorByHash(scope1, itemKind, i0NoId._GetHash());
            auto foundi0 = elems.begin();
            ASSERT_TRUE(foundi0 == elems.end());
            }

        // Write item and the element to which it was "converted"
        if (true)
            {
            iModelBridgeSyncInfoFile::ConversionResults results;
            results.m_element = iModelBridgeTests::CreateGenericPhysicalObject(db);    // it really makes no difference how we create the element.
            ASSERT_EQ(BentleyStatus::SUCCESS, changeDetector._UpdateBimAndSyncInfo(results, change));
            }

        // verify that the item is now in the bim and is unchanged w.r.t. i0NoId
        change = changeDetector._DetectChange(scope1, itemKind, i0NoId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged, change.GetChangeType());

        // Do the same thing, only with a direct query on syncinfo
        if (true)
            {
            auto elems = syncInfo.MakeIteratorByHash(scope1, itemKind, i0NoId._GetHash());
            auto foundi0 = elems.begin();
            ASSERT_TRUE(foundi0 != elems.end());
            ASSERT_TRUE(foundi0.GetSourceState().GetHash().Equals(i0NoId._GetHash()));
            }

        ASSERT_EQ(1, changeDetector.GetElementsConverted());

        ++expected_counts.first;
        ASSERT_EQ(expected_counts, countItemsInSyncInfo(syncInfo));
        ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));
        }

    // Now "change" the input ...
    i0NoId.m_content = "i0NoId changed";

    // Now update the BIM, based on this new input
    if (true)
        {
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(syncInfo);

        // Verify that change detector sees the change and updates the bim and syncinfo
        // Note that, since this item has no ID, it will look like it's new.
        auto change = changeDetector._DetectChange(scope1, itemKind, i0NoId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New, change.GetChangeType());
        if (true)
            {
            iModelBridgeSyncInfoFile::ConversionResults results;
            results.m_element = iModelBridgeTests::CreateGenericPhysicalObject(db);
            ASSERT_EQ(BentleyStatus::SUCCESS, changeDetector._UpdateBimAndSyncInfo(results, change));
            }

        // verify that the item is now in the bim and is unchanged w.r.t. i0NoId
        change = changeDetector._DetectChange(scope1, itemKind, i0NoId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged, change.GetChangeType());

        // Note that the "change" resulted in a *NEW* item. That is how it works for items with no IDs.
        // We have actually added a new element and syncinfo record for the original item and abandoned the first. 
        // It will get cleaned up when we call _DeleteElementsNotSeen later on.
        ++expected_counts.first;
        ASSERT_EQ(expected_counts, countItemsInSyncInfo(syncInfo));
        ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));

        //  Now tell syncinfo to garbage-collect the elements that were abandoned.
        changeDetector._DeleteElementsNotSeen();

        ASSERT_EQ(2, changeDetector.GetElementsConverted()) << "conversion count should be 1 insert + 1 delete";

        //  That should have dropped the count back down to 1 item and 1 element
        --expected_counts.first;
        }

    ASSERT_EQ(expected_counts, countItemsInSyncInfo(syncInfo));
    ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));

    if (true)
        {
        // verify that a second item does not exist in the BIM or in syncinfo
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(syncInfo);
        iModelBridgeSyncInfoFile::ChangeDetector::Results change = changeDetector._DetectChange(scope1, itemKind, i1NoId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New, change.GetChangeType());
        }

    // ------------ ------------------  ------------------  ----------------------  ---------------------
    //  Repeat all that, but using test items that do have IDs
    //      Also, this time, we "skip" scope1 and work only with items in scope2.
    // ------------ ------------------  ------------------  ----------------------  ---------------------

    if (true)
        {
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(syncInfo);
        changeDetector._OnScopeSkipped(scope1);

        // verify that the item does not exist in the BIM or in syncinfo
        iModelBridgeSyncInfoFile::ChangeDetector::Results change = changeDetector._DetectChange(scope2, itemKind, i0WithId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New, change.GetChangeType());

        // verify that second check on the same source item shows the same thing
        change = changeDetector._DetectChange(scope2, itemKind, i0WithId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New, change.GetChangeType());

        // Write item and the element to which it was "converted"
        if (true)
            {
            iModelBridgeSyncInfoFile::ConversionResults results;
            results.m_element = iModelBridgeTests::CreateGenericPhysicalObject(db);    // it really makes no difference how we create the element.
            ASSERT_EQ(BentleyStatus::SUCCESS, changeDetector._UpdateBimAndSyncInfo(results, change));
            }

        // verify that the item is now in the bim and is unchanged w.r.t. i0WithId
        change = changeDetector._DetectChange(scope2, itemKind, i0WithId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged, change.GetChangeType());

        // That added 1 more item to syncinfo and 1 more element to the BIM.
        // It also added an item with an ID, bringing that count up to 1.
        ++expected_counts.first;
        ++expected_counts.second;

        ASSERT_EQ(1, changeDetector.GetElementsConverted());

        //  There should be no garbage
        changeDetector._DeleteElementsNotSeen();    // s/ not do anything.

        ASSERT_EQ(1, changeDetector.GetElementsConverted());
        }

    ASSERT_EQ(expected_counts, countItemsInSyncInfo(syncInfo));
    ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));

    // Now "change" the input ...
    i0WithId.m_content = "i0WithId changed";

    // Now update the BIM, based on this new input
    if (true)
        {
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(syncInfo);
        changeDetector._OnScopeSkipped(scope1);

        // Verify that change detector sees the change and updates the bim and syncinfo
        //      Note that, since this item does have an ID, it will look like it's changed.
        auto change = changeDetector._DetectChange(scope2, itemKind, i0WithId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Changed, change.GetChangeType());
        if (true)
            {
            iModelBridgeSyncInfoFile::ConversionResults results;
            results.m_element = iModelBridgeTests::CreateGenericPhysicalObject(db);
            ASSERT_EQ(BentleyStatus::SUCCESS, changeDetector._UpdateBimAndSyncInfo(results, change));
            }
        // verify that the item is now in the bim and is unchanged w.r.t. i0WithId
        change = changeDetector._DetectChange(scope2, itemKind, i0WithId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged, change.GetChangeType());

        ASSERT_EQ(1, changeDetector.GetElementsConverted());

        // The update should NOT have added a new item or element, so the total should be unchanged.
        }

    ASSERT_EQ(expected_counts, countItemsInSyncInfo(syncInfo));
    ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));

    if (true)
        {
        // verify that a second item does not exist in the BIM or in syncinfo
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(syncInfo);
        changeDetector._OnScopeSkipped(scope1);
        iModelBridgeSyncInfoFile::ChangeDetector::Results change = changeDetector._DetectChange(scope2, itemKind, i1WithId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New, change.GetChangeType());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, iModelBridgeSyncInfoFileTesterSyncInfoFile)
    {
    ScopedDgnHost host;

#ifdef _WIN32
    _set_error_mode(_OUT_TO_MSGBOX);
#endif

    BeFileName bcName;
    GetWriteableCopyOfSeed(bcName, L"Test1.bim");

    iModelBridgeSyncInfoFileTester b;
    iModelBridgeSacAdapter::ParseCommandLineForBeTest(b, {{L"--input=",L"unused"}, {L"--output=",bcName}});

    WCharCP noArgs[] = {L""};

    ASSERT_EQ(BentleyStatus::SUCCESS, b._Initialize(_countof(noArgs), noArgs));

    auto db = DgnDb::OpenDgnDb(nullptr, bcName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    ASSERT_TRUE(db.IsValid());
    auto subj = db->Elements().GetRootSubject();
    ASSERT_TRUE(subj.IsValid());
    // *** NB: the root subject is NOT the correct subject to pass to _ConvertToBim in a real world scenario.
    //          I am just using here, because I need a subject, and it does not matter which, as I am not writing to the BIM in this test.

    ASSERT_EQ(BSISUCCESS, b._OnConvertToBim(*db));  // this is how we pass the Db to the bridge
    ASSERT_EQ(BSISUCCESS, b._OpenSource());

    ASSERT_EQ(BentleyStatus::SUCCESS, b._ConvertToBim(*subj));      // Nearly all of the testing is done in here.

    b._CloseSource(BSISUCCESS);
    b._OnConvertedToBim(BSISUCCESS);

    db->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, FwkArgs)
    {
    bvector<WString> args;
    args.push_back(L"dummyarg0");
    args.push_back(WPrintfString(L"--fwk-staging-dir=\"%ls\"", GetOutputDir().c_str()));
    args.push_back(L"--server-environment=dev");
    args.push_back(L"--server-repository=reponame");
    args.push_back(L"--server-project-guid=projname");
    args.push_back(L"--fwk-create-repository-if-necessary");
    args.push_back(L"--fwk-revision-comment=\"comment in quotes\"");
    args.push_back(L"--server-user=username=username");
    args.push_back(L"--server-password=\"password><!@\"");
    args.push_back(L"--fwk-bridge-regsubkey=regsubkey");
    args.push_back(L"--fwk-input=rootfilename");

    bvector<WCharCP> argptrs;
    for (auto& arg: args)
        argptrs.push_back(arg.c_str());

    iModelBridgeFwk fwk;
    ASSERT_EQ(BSISUCCESS, fwk.ParseCommandLine((int)argptrs.size(), argptrs.data()));

    // Too bad - there's almost nothing we can test in the fwk. We'd need to connect to iModelHub to check the real validity of the parameters.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName getiModelBridgeTestsOutputDir()
    {
    BeFileName testDir;
    BeTest::GetHost().GetOutputRoot(testDir);
    testDir.AppendToPath(L"iModelBridgeTests");
    testDir.AppendToPath(L"Fwk");
    return testDir;
    }

//=======================================================================================
// @bsistruct                                                   Sam.Wilson   10/17
//=======================================================================================
BEGIN_BENTLEY_DGN_NAMESPACE
struct TestiModelHubClient : DgnDbServerClientUtils
{
    BeFileName m_serverRepo;
    DgnDbP m_briefcase;
    struct
        {
        bool haveTxns;
        } m_expect {};


    static BeFileName MakeFakeRepoPath(Utf8CP repoName)
        {
        BeFileName repoPath = getiModelBridgeTestsOutputDir();
        repoPath.AppendToPath(L"Repos");
        repoPath.AppendToPath(WString(repoName,true).c_str());
        return repoPath;
        }

    TestiModelHubClient(WebServices::UrlProvider::Environment environment) : DgnDbServerClientUtils(environment, 0)
        {
        }

    BentleyStatus SignIn(Tasks::AsyncError* servererror, Http::Credentials credentials) override
        {
        return BSISUCCESS;
        }

    BentleyStatus QueryProjectId(WebServices::WSError* wserror, Utf8StringCR bcsProjectName) override
        {
        m_projectId = "Foo";
        return BSISUCCESS;
        }

    void SetProjectId(Utf8CP guid) override {m_projectId=guid;}

    bool IsSignedIn() const override {return true;}

    StatusInt CreateRepository(Utf8CP repoName, BeFileNameCR localDgnDb) override
        {
        m_serverRepo = MakeFakeRepoPath(repoName);
        if (!m_serverRepo.EndsWith(L".bim"))
            m_serverRepo.append(L".bim");
        BeFileName::CreateNewDirectory(m_serverRepo.GetDirectoryName());
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(localDgnDb, m_serverRepo, false));
        return BSISUCCESS;
        }

    StatusInt AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repositoryName) override
        {
        if (m_serverRepo.empty())
            {
            m_lastServerError = iModel::Hub::Error::Id::iModelDoesNotExist;
            return BSIERROR;
            }
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(m_serverRepo, bcFileName, false));

        auto db = DgnDb::OpenDgnDb(nullptr, bcFileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        db->SetAsBriefcase(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
        db->SaveChanges();

        return BSISUCCESS;
        }

    StatusInt OpenBriefcase(Dgn::DgnDbR db) override
        {
        m_briefcase = &db;
        return BSISUCCESS;
        }

    void CloseBriefcase() override
        {
        m_briefcase = nullptr;
        }

    StatusInt PullMergeAndPush(Utf8CP) override
        {
        CaptureChangeSet(m_briefcase);
        return BSISUCCESS;
        }

    void CaptureChangeSet(DgnDbP);

    StatusInt PullAndMerge() override
        {
        return BSISUCCESS;
        }

    StatusInt PullAndMergeSchemaRevisions(Dgn::DgnDbPtr& db) override
        {
        return BSISUCCESS;
        }

    iModel::Hub::Error const& GetLastError() const override
        {
        return m_lastServerError;
        }

    IRepositoryManagerP GetRepositoryManager(DgnDbR db) override
        {
        BeAssert(false); return nullptr;
        }

    StatusInt AcquireLocks(LockRequest&, DgnDbR) override
        {
        return BSISUCCESS;
        }
};
END_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsistruct                                                   Sam.Wilson   04/17
//=======================================================================================
struct iModelBridgeTests_Test1_Bridge : iModelBridgeWithSyncInfoBase
{
    TestSourceItemWithId m_i0;
    TestSourceItemWithId m_i1;
    TestiModelHubClient& m_testIModelHubClient;

    struct
        {
        bool findJobSubject;
        bool anyChanges;
        bool anyDeleted;
        } m_expect {};

    WString _SupplySqlangRelPath() override {return L"sqlang/DgnPlatform_en.sqlang.db3";}
    BentleyStatus _Initialize(int argc, WCharCP argv[]) override {return BSISUCCESS;}
    void _DeleteSyncInfo() override {iModelBridgeSyncInfoFile::DeleteSyncInfoFileFor(_GetParams().GetBriefcaseName());}
    void _OnSourceFileDeleted() override {}

    SubjectCPtr _FindJob() override
        {
        DgnCode jobCode = Subject::CreateCode(*GetDgnDbR().Elements().GetRootSubject(), "iModelBridgeTests_Test1_Bridge");
        auto jobId = GetDgnDbR().Elements().QueryElementIdByCode(jobCode);
        EXPECT_EQ(m_expect.findJobSubject, jobId.IsValid());
        return GetDgnDbR().Elements().Get<Subject>(jobId);
        }

    SubjectCPtr _InitializeJob() override
        {
        EXPECT_TRUE(!m_expect.findJobSubject);

        // Set up the model and category that my superclass's DoTest method uses
        DgnDbTestUtils::InsertPhysicalModel(GetDgnDbR(), "PhysicalModel");
        DgnDbTestUtils::InsertSpatialCategory(GetDgnDbR(), "SpatialCategory");

        auto subjectObj = Subject::Create(*GetDgnDbR().Elements().GetRootSubject(), "iModelBridgeTests_Test1_Bridge");
        return subjectObj->InsertT<Subject>();
        }

    BentleyStatus _ConvertToBim(SubjectCR jobSubject) override
        {
        DoConvertToBim(jobSubject);
        return BentleyStatus::SUCCESS;
        }

    void DoConvertToBim(SubjectCR jobSubject);

    void ConvertItem(TestSourceItemWithId& item, iModelBridgeSyncInfoFile::ChangeDetector&);

    iModelBridgeTests_Test1_Bridge(TestiModelHubClient& tc)
        :
        iModelBridgeWithSyncInfoBase(),
        m_i0("0", "i0WithId initial"),
        m_i1("1", "i1WithId initial"),
        m_testIModelHubClient(tc)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool anyTxnsInFile(DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(db, "SELECT Id FROM " DGN_TABLE_Txns " LIMIT 1");
    return (BE_SQLITE_ROW == stmt.Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TestiModelHubClient::CaptureChangeSet(DgnDbP db)
    {
    ASSERT_TRUE(db != nullptr);

    ASSERT_TRUE(db->IsBriefcase());

    ASSERT_EQ(m_expect.haveTxns, anyTxnsInFile(*db));

    DgnRevisionPtr changeSet = db->Revisions().StartCreateRevision();

    if (!changeSet.IsValid())
        {
        ASSERT_TRUE(!m_expect.haveTxns);
        return;
        }

    ASSERT_TRUE(m_expect.haveTxns);

    ASSERT_TRUE(changeSet.IsValid());
    ASSERT_EQ(Dgn::RevisionStatus::Success, db->Revisions().FinishCreateRevision());
    ASSERT_EQ(BE_SQLITE_OK, db->SaveChanges());

    // *** TBD: test for expected changes
    changeSet->Dump(*db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeTests_Test1_Bridge::ConvertItem(TestSourceItemWithId& item, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector)
    {
    iModelBridgeSyncInfoFile::ROWID scope = 0;  // we don't use scopes in this test
    Utf8CP itemKind = "";   // we don't use kinds in this test

    auto change = changeDetector._DetectChange(scope, itemKind, item);
    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == change.GetChangeType())
        {
        changeDetector._OnElementSeen(change.GetSyncInfoRecord().GetDgnElementId());
        return;
        }
    
    ASSERT_TRUE(m_expect.anyChanges);
    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = iModelBridgeTests::CreateGenericPhysicalObject(*m_db);
    ASSERT_EQ(BentleyStatus::SUCCESS, changeDetector._UpdateBimAndSyncInfo(results, change));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeTests_Test1_Bridge::DoConvertToBim(SubjectCR jobSubject)
    {
    DgnDbR db = *m_db;

    // (Note: superclass iModelBridgeWithSyncInfoBase::_OnConvertToBim has already attached my syncinfo file to the bim.)

    iModelBridgeSyncInfoFile::ChangeDetectorPtr changeDetector = GetSyncInfo().GetChangeDetectorFor(*this);

    // Convert the "items" in my (non-existant) source file.
    ConvertItem(m_i0, *changeDetector);
    ConvertItem(m_i1, *changeDetector);

    //  Garbage-collect the elements that were abandoned.
    changeDetector->_DeleteElementsNotSeen();

    bool anyChanges = (changeDetector->GetElementsConverted() != 0);

    ASSERT_EQ((m_expect.anyChanges || m_expect.anyDeleted), anyChanges);

    m_testIModelHubClient.m_expect.haveTxns = anyChanges;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, Test1)
    {
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(getiModelBridgeTestsOutputDir()));
    
    // I have to create a file that I represent as the bridge "library", so that the fwk's argument validation logic will see that it exists.
    // The fwk won't try to load this file, since we will register a fake bridge.
    BeFileName fakeBridgeName(getiModelBridgeTestsOutputDir());
    fakeBridgeName.AppendToPath(L"iModelBridgeTestsTest1");
    BeFile fakeBridgeFile;
    ASSERT_EQ(BeFileStatus::Success, fakeBridgeFile.Create(fakeBridgeName, true));
    fakeBridgeFile.Close();

    bvector<WString> args;
    args.push_back(L"iModelBridgeTests.Test1");
    args.push_back(WPrintfString(L"--fwk-staging-dir=\"%ls\"", getiModelBridgeTestsOutputDir().c_str()));
    args.push_back(L"--server-environment=Qa");
    args.push_back(L"--server-repository=iModelBridgeTests_Test1");
    args.push_back(L"--server-project-guid=iModelBridgeTests_Project");
    args.push_back(L"--fwk-create-repository-if-necessary");
    args.push_back(L"--fwk-revision-comment=\"comment in quotes\"");
    args.push_back(L"--server-user=username=username");
    args.push_back(L"--server-password=\"password><!@\"");
    args.push_back(WPrintfString(L"--fwk-bridge-library=\"%ls\"", fakeBridgeName.c_str()));
    BeFileName platformAssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
    args.push_back(WPrintfString(L"--fwk-bridgeAssetsDir=\"%ls\"", platformAssetsDir.c_str())); // the platform's assets dir will serve just find as the test bridge's assets dir.
    args.push_back(L"--fwk-input=Foo");

    bvector<WCharCP> argptrs;
    for (auto& arg: args)
        argptrs.push_back(arg.c_str());

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestiModelHubClient testClient(WebServices::UrlProvider::Environment::Qa);
    iModelBridgeFwk::SetDgnDbServerClientUtilsForTesting(testClient);

    // Register the test bridge that fwk should run
    iModelBridgeTests_Test1_Bridge testBridge(testClient);
    iModelBridgeFwk::SetBridgeForTesting(testBridge);

    int argc = (int)argptrs.size();
    wchar_t const** argv = argptrs.data();
    if (true)
        {
        testClient.m_expect.haveTxns = false; // Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = false;
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = false;
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }

    if (true)
        {
        // Modify an item 
        testBridge.m_i0.m_content = "changed";

        // and run an update
        // This time, we expect to find the repo and briefcase already there.
        testClient.m_expect.haveTxns = false; // Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = false;
        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }

    if (true)
        {
        // Run an update with no changes
        testClient.m_expect.haveTxns = false; // Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = false;
        testBridge.m_expect.anyDeleted = false;
        iModelBridgeFwk fwk;
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }
    }

#include "../../Fwk/DgnDbServerClientUtils.cpp"
