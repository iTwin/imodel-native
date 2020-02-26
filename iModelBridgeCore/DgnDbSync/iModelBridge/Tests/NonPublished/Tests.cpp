/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/SHA1.h>
#include <iModelBridge/iModelBridge.h>
#include <iModelBridge/iModelBridgeSyncInfoFile.h>
#include <iModelBridge/iModelBridgeSacAdapter.h>
#include <iModelBridge/iModelBridgeFwk.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <DgnPlatform/GenericDomain.h>
#include <iModelBridge/IModelClientForBridges.h>
#include <Bentley/BeFileName.h>
#include <iModelBridge/FakeRegistry.h>
#include <iModelBridge/TestiModelHubClientForBridges.h>
#include <iModelBridge/iModelBridgeLdClient.h>
#include <PlacementonEarth/Placement.h>
#include <csignal>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

#define MAKE_ARGC_ARGV(argptrs, args)\
for (auto& arg: args)\
   argptrs.push_back(arg.c_str());\
int argc = (int)argptrs.size();\
wchar_t const** argv = argptrs.data();\

static Utf8CP s_fooGuid = "6640b375-a539-4e73-b3e1-2c0ceb912551";
static Utf8CP s_barGuid = "6640b375-a539-4e73-b3e1-2c0ceb912552";

static int s_sigReceived;
static void testSignalHandler(int s)
    {
    s_sigReceived = s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<SubjectCPtr> getJobSubjects(DgnDbR db)
    {
	bvector<SubjectCPtr> subjects;
    EC::ECSqlStatement stmt;
    stmt.Prepare(db, "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_Subject) " WHERE json_extract(JsonProperties, '$.Subject.Job') is not null");
    while (BE_SQLITE_ROW != stmt.Step())
        {
        subjects.push_back(db.Elements().Get<Subject>(stmt.GetValueId<DgnElementId>(0)));
        }
    return subjects;
    }

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
struct iModelBridgeSyncInfoFileTester : iModelBridgeWithSyncInfoBase
{
    WString _SupplySqlangRelPath() override {return L"sqlang/DgnPlatform_en.sqlang.db3";}
    BentleyStatus _Initialize(int argc, WCharCP argv[]) override {return BSISUCCESS;}
    BentleyStatus _ConvertToBim(SubjectCR jobSubject) override {DoTests(jobSubject); return BentleyStatus::SUCCESS;}
    SubjectCPtr _FindJob() override {return nullptr;}
    SubjectCPtr _InitializeJob() override {return nullptr;}
    BentleyStatus _DetectDeletedDocuments() override { return BSISUCCESS;}

    void DoTests(SubjectCR jobSubject);

    iModelBridgeSyncInfoFileTester() : iModelBridgeWithSyncInfoBase() {}

    void _OnDocumentDeleted(Utf8StringCR docId, iModelBridgeSyncInfoFile::ROWID docrid) override
        {
        Utf8String ecsql("SELECT ECInstanceId, Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Scope.Id=?)");

        auto stmt = GetDgnDbR().GetPreparedECSqlStatement(ecsql.c_str());

        if (!stmt.IsValid())
            return;

        stmt->BindId(1, BeInt64Id(docrid));
       
        while  (BeSQLite::BE_SQLITE_ROW == stmt->Step())
            {
            DgnElementId id = stmt->GetValueId<DgnElementId>(1);
            if (!id.IsValid())
                continue; // not every record in syncinfo is an element
            auto el = GetDgnDbR().Elements().GetElement(id);
            ASSERT_TRUE(el.IsValid());
            el->Delete();
            }
        }

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
    Utf8String m_type;
    Placement3d m_placement;
    TestSourceItemWithId(Utf8StringCR id, Utf8StringCR content, Utf8StringCR type, Placement3d placement) : m_id(id), m_content(content), m_type(type) {}
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

    Utf8String ecsql("SELECT ECInstanceId, Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect));
    auto stmt = syncInfo.GetDgnDb().GetPreparedECSqlStatement(ecsql.c_str());
    if (!stmt.IsValid())
        return make_bpair(count, countThoseWithIds);

    while (BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        ++count;
        DgnElementId id = stmt->GetValueId<DgnElementId>(1);
        if (!id.IsValid())
            continue; // not every record in syncinfo is an element
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
    ASSERT_EQ(BentleyStatus::SUCCESS, m_syncInfo.AttachToBIM(db));

    // Items in scope1
    iModelBridgeSyncInfoFile::ROWID scope1;
    TestSourceItemNoId i0NoId("i0NoId initial");
    TestSourceItemNoId i1NoId("i1NoId initial");
    // Items in scope2
    iModelBridgeSyncInfoFile::ROWID scope2;
    TestSourceItemWithId i0WithId("0", "i0WithId initial", "Foo", Placement3d());
    TestSourceItemWithId i1WithId("1", "i1WithId initial", "Foo", Placement3d());


    Utf8CP itemKind = "";

    if (true)
        {
        iModelBridgeSyncInfoFile::ChangeDetector nullChangeDetector(db);
        // put the two scope items into syncinfo to start with.
        // Note that I don't have to create elements in order to put records into syncinfo. These records will
        // just serve as "scopes" to partition the items that I will "convert" below.
        iModelBridgeSyncInfoFile::SourceState noState(0.0, "");;
        iModelBridgeSyncInfoFile::ConversionResults docLink1 = RecordDocument(*GetSyncInfo().GetChangeDetectorFor(*this), BeFileName( L"First One"), nullptr,
            "DocumentWithBeGuid", iModelBridgeSyncInfoFile::ROWID(jobSubject.GetElementId().GetValue()));
        
        scope1 = docLink1.m_syncInfoRecord.GetROWID();
        iModelBridgeSyncInfoFile::ConversionResults docLink2 = RecordDocument(*GetSyncInfo().GetChangeDetectorFor(*this), BeFileName(L"Second One"), nullptr,
            "DocumentWithBeGuid", iModelBridgeSyncInfoFile::ROWID(jobSubject.GetElementId().GetValue()));
        scope2 = docLink2.m_syncInfoRecord.GetROWID();
        }

    bvector<iModelBridgeSyncInfoFile::ROWID> scopes = {scope1, scope2};

    auto expected_counts = make_bpair<size_t,size_t>(2,2);  // (total count, count of items with IDs)

    if (true)
        {
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(db);

        // verify that the item does not exist in the BIM or in syncinfo
        iModelBridgeSyncInfoFile::ChangeDetector::Results change = changeDetector._DetectChange(scope1, itemKind, i0NoId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New, change.GetChangeType());
        ASSERT_FALSE(change.IsItemStale());

        // verify that second check on the same source item shows the same thing
        change = changeDetector._DetectChange(scope1, itemKind, i0NoId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New, change.GetChangeType());
        ASSERT_FALSE(change.IsItemStale());

        ASSERT_EQ(expected_counts, countItemsInSyncInfo(m_syncInfo));
        ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));

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
        ASSERT_FALSE(change.IsItemStale());

        ASSERT_EQ(1, changeDetector.GetElementsConverted());

        //  Now tell syncinfo to garbage-collect the elements that were abandoned.
        changeDetector._DeleteElementsNotSeenInScopes(scopes);

        ASSERT_EQ(1, changeDetector.GetElementsConverted()) << "conversion count should be 1 insert";

        ++expected_counts.first;
        ++expected_counts.second;
        ASSERT_EQ(expected_counts, countItemsInSyncInfo(m_syncInfo));
        ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));
        }

    // Now "change" the input ...
    i0NoId.m_content = "i0NoId changed";

    // Now update the BIM, based on this new input
    if (true)
        {
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(db);

        // Verify that change detector sees the change and updates the bim and syncinfo
        // Note that, since this item has no ID, it will look like it's new.
        auto change = changeDetector._DetectChange(scope1, itemKind, i0NoId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New, change.GetChangeType());
        ASSERT_FALSE(change.IsItemStale());
        if (true)
            {
            iModelBridgeSyncInfoFile::ConversionResults results;
            results.m_element = iModelBridgeTests::CreateGenericPhysicalObject(db);
            ASSERT_EQ(BentleyStatus::SUCCESS, changeDetector._UpdateBimAndSyncInfo(results, change));
            }

        // verify that the item is now in the bim and is unchanged w.r.t. i0NoId
        change = changeDetector._DetectChange(scope1, itemKind, i0NoId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged, change.GetChangeType());
        ASSERT_FALSE(change.IsItemStale());

        // Note that the "change" resulted in a *NEW* item. That is how it works for items with no IDs.
        // We have actually added a new element and syncinfo record for the original item and abandoned the first. 
        // It will get cleaned up when we call DeleteElementsNotSeenInScope later on.
        ++expected_counts.first;
        ++expected_counts.second;
        ASSERT_EQ(expected_counts, countItemsInSyncInfo(m_syncInfo));
        ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));

        //  Now tell syncinfo to garbage-collect the elements that were abandoned.
        changeDetector._DeleteElementsNotSeenInScopes(scopes);

        ASSERT_EQ(2, changeDetector.GetElementsConverted()) << "conversion count should be 1 insert + 1 delete";

        //  That should have dropped the count back down to 1 item and 1 element
        --expected_counts.first;
        --expected_counts.second;
        }

    ASSERT_EQ(expected_counts, countItemsInSyncInfo(m_syncInfo));
    ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));

    if (true)
        {
        // verify that a second item does not exist in the BIM or in syncinfo
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(db);
        iModelBridgeSyncInfoFile::ChangeDetector::Results change = changeDetector._DetectChange(scope1, itemKind, i1NoId);
        ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New, change.GetChangeType());
        }

    // ------------ ------------------  ------------------  ----------------------  ---------------------
    //  Repeat all that, but using test items that do have IDs
    //      Also, this time, we "skip" scope1 and work only with items in scope2.
    // ------------ ------------------  ------------------  ----------------------  ---------------------

    if (true)
        {
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(db);
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
        changeDetector._DeleteElementsNotSeenInScopes(scopes);    // s/ not do anything.

        ASSERT_EQ(1, changeDetector.GetElementsConverted());
        }

    ASSERT_EQ(expected_counts, countItemsInSyncInfo(m_syncInfo));
    ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));

    // Now "change" the input ...
    i0WithId.m_content = "i0WithId changed";

    // Now update the BIM, based on this new input
    if (true)
        {
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(db);
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

    ASSERT_EQ(expected_counts, countItemsInSyncInfo(m_syncInfo));
    ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));

    if (true)
        {
        // verify that a second item does not exist in the BIM or in syncinfo
        iModelBridgeSyncInfoFile::ChangeDetector changeDetector(db);
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
    GetWriteableCopyOfSeed(bcName, L"iModelBridgeSyncInfoFileTesterSyncInfoFile.bim");

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

    ASSERT_EQ(BSISUCCESS, b._OnOpenBim(*db));  // this is how we pass the Db to the bridge
    ASSERT_EQ(BSISUCCESS, b._OpenSource());

    ASSERT_EQ(BentleyStatus::SUCCESS, b._ConvertToBim(*subj));      // Nearly all of the testing is done in here.

    b._CloseSource(BSISUCCESS, iModelBridge::ClosePurpose::Finished);
    b._OnCloseBim(BSISUCCESS, iModelBridge::ClosePurpose::Finished);
    b._Terminate(BSISUCCESS);

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
    //args.push_back(L"--fwk-bridge-regsubkey=regsubkey"); // CGM - Trying to isolate where firebug is failing in these tests
    //args.push_back(L"--fwk-input=rootfilename");
    args.push_back(L"--fwk-ignore-stale-files");
    args.push_back(L"--fwk-error-on-stale-files");

    bvector<WCharCP> argptrs;
    for (auto& arg: args)
        argptrs.push_back(arg.c_str());

    iModelBridgeFwk fwk;
    ASSERT_EQ(BSISUCCESS, fwk.ParseCommandLine((int)argptrs.size(), argptrs.data()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName getiModelBridgeTestsOutputDir(WCharCP subdir)
    {
    BeFileName testDir;
    BeTest::GetHost().GetOutputRoot(testDir);
    testDir.AppendToPath(L"iModelBridgeTests");
    testDir.AppendToPath(L"Fwk");
    testDir.AppendToPath(subdir);
    return testDir;
    }

//=======================================================================================
// @bsistruct                                                   Sam.Wilson   10/17
//=======================================================================================
BEGIN_BENTLEY_DGN_NAMESPACE
struct TestIModelHubFwkClientForBridges : TestIModelHubClientForBridges
    {
    std::deque < bool> m_expect;
    
    TestIModelHubFwkClientForBridges(BeFileNameCR testWorkDir) 
        : TestIModelHubClientForBridges(testWorkDir)
        {  
        
        }

    iModel::Hub::iModelInfoPtr GetIModelInfo() override { return m_iModelInfo; }
        virtual DgnRevisionPtr CaptureChangeSet(DgnDbP db, Utf8CP comment) override;
    };
END_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsistruct                                                   Sam.Wilson   04/17
//=======================================================================================
struct iModelBridgeTests_Test1_Bridge : iModelBridgeWithSyncInfoBase
{
    std::vector <TestSourceItemWithId> m_foo_items;
    
    TestIModelHubFwkClientForBridges& m_testIModelHubClientForBridges;
    iModelBridgeSyncInfoFile::ROWID m_docScopeId;
    bool m_jobTransChanged = false;
    int m_changeCount = 0;

    struct
        {
        bool findJobSubject = false;
        bool anyChanges = false;
        bool anyDeleted = false;
        bool isDocumentDeletedCase = false;
        bool assignmentCheck = false;
        bool jobTransChanged = false;
        bvector<Utf8String> docsDeleted;
        } m_expect;

    Utf8String ComputeJobSubjectCodeValue()
        {
        // note that the jobs are root specific and so the job subject's code must include the input filename
        Utf8String jobCode("iModelBridgeTests_Test1_Bridge - ");
        jobCode.append(Utf8String(_GetParams().GetInputFileName()).c_str());
        return jobCode;
        }

    WString _SupplySqlangRelPath() override {return L"sqlang/DgnPlatform_en.sqlang.db3";}
    BentleyStatus _Initialize(int argc, WCharCP argv[]) override {return BSISUCCESS;}

    void _OnDocumentDeleted(Utf8StringCR docId, iModelBridgeSyncInfoFile::ROWID docrid) override
        {
        ASSERT_TRUE(std::find(m_expect.docsDeleted.begin(), m_expect.docsDeleted.end(), docId) != m_expect.docsDeleted.end());

        Utf8String ecsql("SELECT ECInstanceId, Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ExternalSourceAspect) " WHERE (Scope.Id=?)");

        auto stmt = GetDgnDbR().GetPreparedECSqlStatement(ecsql.c_str());

        if (!stmt.IsValid())
            return;

        stmt->BindId(1, BeInt64Id(docrid));
       
        while  (BeSQLite::BE_SQLITE_ROW == stmt->Step())
            {
            ASSERT_TRUE(m_expect.anyDeleted);
            DgnElementId id = stmt->GetValueId<DgnElementId>(1);
            if (!id.IsValid())
                continue; // not every record in syncinfo is an element
            auto el = GetDgnDbR().Elements().GetElement(id);
            ASSERT_TRUE(el.IsValid());
            el->Delete();
            }
    
        m_testIModelHubClientForBridges.m_expect.push_back(m_expect.anyDeleted);
        }

    SubjectCPtr _FindJob() override
        {
        DgnCode jobCode = Subject::CreateCode(*GetDgnDbR().Elements().GetRootSubject(), ComputeJobSubjectCodeValue().c_str());
        auto jobId = GetDgnDbR().Elements().QueryElementIdByCode(jobCode);
        EXPECT_EQ(m_expect.findJobSubject, jobId.IsValid());
        return GetDgnDbR().Elements().Get<Subject>(jobId);
        }
     
    SubjectCPtr _InitializeJob() override
        {
        EXPECT_TRUE(!m_expect.findJobSubject);

        
        // Set up the model and category that my superclass's DoTest method uses
        DgnCode partitionCode = PhysicalPartition::CreateCode(*GetDgnDbR().Elements().GetRootSubject(), "PhysicalModel");
        if (!GetDgnDbR().Elements().QueryElementIdByCode(partitionCode).IsValid())
            {
            DgnDbTestUtils::InsertPhysicalModel(GetDgnDbR(), "PhysicalModel");
            DgnDbTestUtils::InsertSpatialCategory(GetDgnDbR(), "SpatialCategory");
            }

        auto subjectObj = Subject::Create(*GetDgnDbR().Elements().GetRootSubject(), ComputeJobSubjectCodeValue().c_str());
        JobSubjectUtils::InitializeProperties(*subjectObj, _GetParams().GetBridgeRegSubKeyUtf8());
        m_testIModelHubClientForBridges.m_expect.push_back(true);
        m_expect.findJobSubject = true;

        SubjectCPtr subj =  subjectObj->InsertT<Subject>();

        // register the document. This then becomes the scope for all of my items.
        iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(*GetSyncInfo().GetChangeDetectorFor(*this), _GetParams().GetInputFileName(), nullptr,
            "DocumentWithBeGuid", iModelBridgeSyncInfoFile::ROWID(subj->GetElementId().GetValue()));
        
        return subj;
        }

    BentleyStatus _ConvertToBim(SubjectCR jobSubject) override
        {
        DoConvertToBim(jobSubject);
        return BentleyStatus::SUCCESS;
        }

    void DoConvertToBim(SubjectCR jobSubject);

    void ConvertItem(TestSourceItemWithId& item, iModelBridgeSyncInfoFile::ChangeDetector&);

    iModelBridgeTests_Test1_Bridge(TestIModelHubFwkClientForBridges& tc)
        :
        iModelBridgeWithSyncInfoBase(),
        m_testIModelHubClientForBridges(tc)
        {
        YawPitchRollAngles angles;
        //double left, double front, double bottom, double right, double back, double top
        ElementAlignedBox3d range(10, 10, 10, 10, 10, 10);
        Placement3d placement(DPoint3d::FromZero(), angles, range);
        m_foo_items.push_back(TestSourceItemWithId("0", "foo i0 - initial", "Foo",  placement));
        placement.GetOriginR().x += 100;
        m_foo_items.push_back(TestSourceItemWithId("1", "foo i1 - initial", "Foo", placement));
        placement.GetOriginR().y += 100;
        m_foo_items.push_back(TestSourceItemWithId("0", "bar i0 - initial", "Bar", placement));
        placement.GetOriginR().x -= 100;
        m_foo_items.push_back(TestSourceItemWithId("1", "bar i1 - initial", "Bar", placement));
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void populateRegistryWithFooBar(FakeRegistry& testRegistry, WString bridgeRegSubKey)
    {
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer,
                                                          const size_t bufferSize,
                                                          BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel,
                                                          BentleyApi::WCharCP affinityLibraryPath,
                                                          BentleyApi::WCharCP sourceFileName)
        {
        affinityLevel = iModelBridgeAffinityLevel::Medium;
        wcsncpy(buffer, bridgeRegSubKey.c_str(), bridgeRegSubKey.length());
        };

    testRegistry.AddBridge(bridgeRegSubKey, lambda);

    iModelBridgeDocumentProperties fooDocProps(s_fooGuid, "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties barDocProps(s_barGuid, "wurn2", "durn2", "other2", "");
    testRegistry.SetDocumentProperties(fooDocProps, BeFileName(L"Foo"));
    testRegistry.SetDocumentProperties(barDocProps, BeFileName(L"Bar"));
    WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName,BeFileName(L"Foo"),L"");
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName,BeFileName(L"Bar"),L"");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRevisionPtr TestIModelHubFwkClientForBridges::CaptureChangeSet(DgnDbP db, Utf8CP comment)
    {
    BeAssert(db != nullptr);

    BeAssert(db->IsBriefcase());
    bool expectedResult = false;
    if (!m_expect.empty())
        {
        expectedResult = m_expect.front();
        m_expect.pop_front();
        }
    else
        {
        fprintf(stderr, "m_expect is empty. Push comment = \"%s\"", comment);
        BeAssert(false && "m_expect is empty");
        }

    BeAssert(expectedResult == anyTxnsInFile(*db));

    // printf("*** %s : %d\n", comment, anyTxnsInFile(*db));

    if (!anyTxnsInFile(*db))
        return nullptr;

    DgnRevisionPtr changeSet = db->Revisions().StartCreateRevision();

    if (!changeSet.IsValid())
        {
        db->Revisions().FinishCreateRevision();
        BeAssert(!expectedResult);
        return changeSet;
        }

    if (comment)
        changeSet->SetSummary(comment);

    BeAssert(expectedResult);

    BeAssert(changeSet.IsValid());
    BeAssert(Dgn::RevisionStatus::Success ==  db->Revisions().FinishCreateRevision());
    BeAssert(BE_SQLITE_OK == db->SaveChanges());

    // *** TBD: test for expected changes
    changeSet->Dump(*db);
    return changeSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeTests_Test1_Bridge::ConvertItem(TestSourceItemWithId& item, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector)
    {
    Utf8CP itemKind = "ItemKind";   // we don't use kinds in this test

    auto change = changeDetector._DetectChange(m_docScopeId, itemKind, item, nullptr, m_jobTransChanged);
    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == change.GetChangeType())
        {
        changeDetector._OnElementSeen(change.GetSyncInfoRecord().GetDgnElementId());
        return;
        }
    
    ++m_changeCount;
    ASSERT_TRUE(m_expect.anyChanges);
    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = iModelBridgeTests::CreateGenericPhysicalObject(*m_db);
    DgnElementTransformer::ApplyTransformTo(*results.m_element, GetSpatialDataTransform());
    ASSERT_EQ(BentleyStatus::SUCCESS, changeDetector._UpdateBimAndSyncInfo(results, change));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeTests_Test1_Bridge::DoConvertToBim(SubjectCR jobSubject)
    {
    // (Note: superclass iModelBridgeWithSyncInfoBase::_OnConvertToBim has already attached my syncinfo file to the bim.)

    if (m_expect.isDocumentDeletedCase)
        {
        _DetectDeletedDocuments();
        return;
        }

    if (m_expect.assignmentCheck)
        {
        ASSERT_TRUE(IsFileAssignedToBridge(_GetParams().GetInputFileName()));
        }

    iModelBridgeSyncInfoFile::ChangeDetectorPtr changeDetector = GetSyncInfo().GetChangeDetectorFor(*this);
    iModelBridgeSyncInfoFile::ChangeDetector::Results change;
    iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(change, *changeDetector, _GetParams().GetInputFileName(), nullptr,
        "DocumentWithBeGuid", iModelBridgeSyncInfoFile::ROWID(jobSubject.GetElementId().GetValue()));

    ASSERT_EQ(iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged, change.GetChangeType());

    m_docScopeId = docLink.m_element->GetElementId().GetValue();

    Transform _newTrans, _oldTrans;
    m_jobTransChanged = DetectSpatialDataTransformChange(_newTrans, _oldTrans, *changeDetector, m_docScopeId, "JT", "JT");
    ASSERT_EQ(m_expect.jobTransChanged, m_jobTransChanged);

    // Convert the "items" in my (non-existant) source file.
    for (auto item : m_foo_items)
        {
        if (item.m_type.EqualsI(Utf8String(_GetParams().GetInputFileName())))
            ConvertItem(item, *changeDetector);
        }

    //  Garbage-collect the elements that were abandoned.
    changeDetector->DeleteElementsNotSeenInScope(m_docScopeId);

    bool anyChanges = (changeDetector->GetElementsConverted() != 0);

    ASSERT_EQ((m_expect.anyChanges || m_expect.anyDeleted), anyChanges);

    m_testIModelHubClientForBridges.m_expect.push_back(anyChanges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, Test1)
    {
    auto bridgeRegSubKey = L"iModelBridgeTests_Test1_Bridge";

    auto testDir = getiModelBridgeTestsOutputDir(L"Test1");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));
    
    // I have to create a file that I represent as the bridge "library", so that the fwk's argument validation logic will see that it exists.
    // The fwk won't try to load this file, since we will register a fake bridge.
    BeFileName fakeBridgeName(testDir);
    fakeBridgeName.AppendToPath(L"iModelBridgeTests-Test1");
    BeFile fakeBridgeFile;
    ASSERT_EQ(BeFileStatus::Success, fakeBridgeFile.Create(fakeBridgeName, true));
    fakeBridgeFile.Close();

    bvector<WString> args;
    args.push_back(L"iModelBridgeTests.Test1");                                                 // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir.c_str()));
    args.push_back(L"--server-environment=Qa");
    args.push_back(L"--server-repository=iModelBridgeTests_Test1");                             // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-project-guid=iModelBridgeTests_Project");                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--fwk-create-repository-if-necessary");
    args.push_back(L"--fwk-revision-comment=\"comment in quotes\"");
    args.push_back(L"--server-user=username=username");                                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-password=\"password><!@\"");                                      // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-bridge-library=\"%ls\"", fakeBridgeName.c_str()));     // must refer to a path that exists! 
    args.push_back(WPrintfString(L"--fwk-bridge-regsubkey=%ls", bridgeRegSubKey).c_str());      // must be consistent with testRegistry.m_bridgeRegSubKey
    BeFileName platformAssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
    args.push_back(WPrintfString(L"--fwk-bridgeAssetsDir=\"%ls\"", platformAssetsDir.c_str())); // must be a real assets dir! the platform's assets dir will serve just find as the test bridge's assets dir.
    args.push_back(L"--fwk-input=Foo");

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubFwkClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);

    // Register the test bridge that fwk should run
    iModelBridgeTests_Test1_Bridge testBridge(testIModelHubClientForBridges);
    iModelBridgeFwk::SetBridgeForTesting(testBridge);

    iModelBridgeFwk::SetSignalHandlerForTesting(testSignalHandler);

    BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"test1Assignments.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    populateRegistryWithFooBar(testRegistry, bridgeRegSubKey);
    
    testRegistry.AddRef(); // prevent ~iModelBridgeFwk from deleting this object.
    iModelBridgeFwk::SetRegistryForTesting(testRegistry);   // (takes ownership of pointer)

    if (true)
        {
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = false;
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = false;
        testIModelHubClientForBridges.m_expect.push_back(true);//For project extents
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        testIModelHubClientForBridges.m_expect.clear();

        ASSERT_EQ(s_sigReceived, 0);
        std::raise(SIGINT);
        ASSERT_EQ(s_sigReceived, SIGINT);
        }

    if (true)
        {
        // Modify an item 
        testBridge.m_foo_items[0].m_content = "changed";

        // and run an update
        // This time, we expect to find the repo and briefcase already there.
        testIModelHubClientForBridges.m_expect.push_back(false);// iModelBridgeTests_Test1_Bridge - Foo (6640b375-a539-4e73-b3e1-2c0ceb912551) -  : 0
        testIModelHubClientForBridges.m_expect.push_back(true);// iModelBridgeTests_Test1_Bridge - Foo (6640b375-a539-4e73-b3e1-2c0ceb912551) - dynamic schemas : 1
        testIModelHubClientForBridges.m_expect.push_back(true);// iModelBridgeTests_Test1_Bridge - Foo (6640b375-a539-4e73-b3e1-2c0ceb912551) - definitions : 1
        testIModelHubClientForBridges.m_expect.push_back(true);// iModelBridgeTests_Test1_Bridge - Foo (6640b375-a539-4e73-b3e1-2c0ceb912551) - comment in quotes : 1
        testIModelHubClientForBridges.m_expect.push_back(true);//For project extents
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = false;
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        testIModelHubClientForBridges.m_expect.clear();
        }

    if (true)
        {
        // Run an update with no changes
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = false;
        testBridge.m_expect.anyDeleted = false;
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }

    if (true)
    {
        // Run an update with deleting an item.
        //testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testIModelHubClientForBridges.m_expect.push_back(true);// iModelBridgeTests_Test1_Bridge - Foo (6640b375-a539-4e73-b3e1-2c0ceb912551) - delete
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = true;
        testBridge.m_foo_items.erase(testBridge.m_foo_items.begin());
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, DelDocTest1)
    {
    auto bridgeRegSubKey = L"iModelBridgeTests_Test1_Bridge";

    auto testDir = getiModelBridgeTestsOutputDir(L"DelDocTest1");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));
    
    // I have to create a file that I represent as the bridge "library", so that the fwk's argument validation logic will see that it exists.
    // The fwk won't try to load this file, since we will register a fake bridge.
    BeFileName fakeBridgeName(testDir);
    fakeBridgeName.AppendToPath(L"iModelBridgeTests-DelDocTest1");
    BeFile fakeBridgeFile;
    ASSERT_EQ(BeFileStatus::Success, fakeBridgeFile.Create(fakeBridgeName, true));
    fakeBridgeFile.Close();

    bvector<WString> args;
    args.push_back(L"iModelBridgeTests.DelDocTest1");                       // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir.c_str()));
    args.push_back(L"--server-environment=Qa");
    args.push_back(L"--server-repository=iModelBridgeTests_DelDocTest1");   // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-project-guid=iModelBridgeTests_Project");     // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--fwk-create-repository-if-necessary");        
    args.push_back(L"--server-user=username=username");                     // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-password=\"password><!@\"");                  // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-bridge-regsubkey=%ls", bridgeRegSubKey).c_str());  // must be consistent with testRegistry.m_bridgeRegSubKey
    args.push_back(WPrintfString(L"--fwk-bridge-library=\"%ls\"", fakeBridgeName.c_str())); // must refer to a path that exists! 
    BeFileName platformAssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
    args.push_back(WPrintfString(L"--fwk-bridgeAssetsDir=\"%ls\"", platformAssetsDir.c_str())); // must be a real assets dir! the platform's assets dir will serve just find as the test bridge's assets dir.

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubFwkClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);

    // Register the test bridge that fwk should run
    iModelBridgeTests_Test1_Bridge testBridge(testIModelHubClientForBridges);
    iModelBridgeFwk::SetBridgeForTesting(testBridge);

    BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"DelDocTest1.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    populateRegistryWithFooBar(testRegistry, bridgeRegSubKey);
    testRegistry.AddRef(); // prevent ~iModelBridgeFwk from deleting this object.
    iModelBridgeFwk::SetRegistryForTesting(testRegistry);   // (takes ownership of pointer)

    testBridge.m_expect.assignmentCheck = true;

    if (true)
        {
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = false;
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = false;
        testIModelHubClientForBridges.m_expect.push_back(true);//For project extents
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        testIModelHubClientForBridges.m_expect.clear();
        }

    if (true)
        {
        // convert another document
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = false; // since this is a new "root" document, it must have its own jobsubject
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = false;
        testIModelHubClientForBridges.m_expect.push_back(true);//For project extents
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Bar");
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        testIModelHubClientForBridges.m_expect.clear();
        }

    if (true)
        {
        // Run an update with no changes
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testIModelHubClientForBridges.m_expect.push_back(false);
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = false;
        testBridge.m_expect.anyDeleted = false;
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        testIModelHubClientForBridges.m_expect.clear();
        }

    if (true)
        {
        // now pretend that the document called "bar" was deleted.
        testRegistry.RemoveFileAssignment(BeFileName(L"Bar"));
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = false;
        testBridge.m_expect.anyDeleted = true;
        testBridge.m_expect.isDocumentDeletedCase = true;
        testBridge.m_expect.docsDeleted.push_back(s_barGuid);
        testIModelHubClientForBridges.m_expect.push_back(true);//For project extents
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Bar");
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        testIModelHubClientForBridges.m_expect.clear();
        }

    if (true)
        {
        // Run an update with no changes
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = false;
        testBridge.m_expect.anyDeleted = false;
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        testIModelHubClientForBridges.m_expect.clear();
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, SpatialDataTransformTest)
    {
    auto bridgeRegSubKey = L"iModelBridgeTests_Test1_Bridge";

    auto testDir = getiModelBridgeTestsOutputDir(L"SpatialDataTransformTest");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));
    
    // I have to create a file that I represent as the bridge "library", so that the fwk's argument validation logic will see that it exists.
    // The fwk won't try to load this file, since we will register a fake bridge.
    BeFileName fakeBridgeName(testDir);
    fakeBridgeName.AppendToPath(L"iModelBridgeTests-SpatialDataTransformTest");
    BeFile fakeBridgeFile;
    ASSERT_EQ(BeFileStatus::Success, fakeBridgeFile.Create(fakeBridgeName, true));
    fakeBridgeFile.Close();

    bvector<WString> args;
    args.push_back(L"iModelBridgeTests.SpatialDataTransformTest");                       // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir.c_str()));
    args.push_back(L"--server-environment=Qa");
    args.push_back(L"--server-repository=iModelBridgeTests_SpatialDataTransformTest");   // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-project-guid=iModelBridgeTests_Project");     // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--fwk-create-repository-if-necessary");        
    args.push_back(L"--server-user=username=username");                     // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-password=\"password><!@\"");                  // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-bridge-regsubkey=%ls", bridgeRegSubKey).c_str());  // must be consistent with testRegistry.m_bridgeRegSubKey
    args.push_back(WPrintfString(L"--fwk-bridge-library=\"%ls\"", fakeBridgeName.c_str())); // must refer to a path that exists! 
    BeFileName platformAssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
    args.push_back(WPrintfString(L"--fwk-bridgeAssetsDir=\"%ls\"", platformAssetsDir.c_str())); // must be a real assets dir! the platform's assets dir will serve just find as the test bridge's assets dir.

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubFwkClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);

    // Register the test bridge that fwk should run
    iModelBridgeTests_Test1_Bridge testBridge(testIModelHubClientForBridges);
    iModelBridgeFwk::SetBridgeForTesting(testBridge);

    
    BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"SpatialDataTransformTest.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    populateRegistryWithFooBar(testRegistry, bridgeRegSubKey);
    testRegistry.AddRef(); // prevent ~iModelBridgeFwk from deleting this object.
    iModelBridgeFwk::SetRegistryForTesting(testRegistry);   // (takes ownership of pointer)
    
    
    

    testBridge.m_expect.assignmentCheck = true;

    Json::Value transform_offset1;
    iModelBridge::Params::SetOffsetJson(transform_offset1, DPoint3d::From(1,0,0), AngleInDegrees::FromDegrees(0.0));
    WPrintfString transform_offset1_str(L"--fwk-argsJson=\"%s\"", WString(transform_offset1.ToString().c_str(), true).c_str());
    Json::Value transform_offset1_45;
    iModelBridge::Params::SetOffsetJson(transform_offset1_45, DPoint3d::From(1,0,0), AngleInDegrees::FromDegrees(45.0));

    if (true)
        {
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = false;
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = false;
        testBridge.m_expect.jobTransChanged = false;
        testIModelHubClientForBridges.m_expect.push_back(true);//For project extents
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        testIModelHubClientForBridges.m_expect.clear();
        }
    
    if (true)
        {
        // Run an update with no changes
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testIModelHubClientForBridges.m_expect.push_back(true);
        testIModelHubClientForBridges.m_expect.push_back(false);
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = false;
        testBridge.m_expect.anyDeleted = false;
        testBridge.m_expect.jobTransChanged = false;
        testBridge.m_changeCount = 0;
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        EXPECT_EQ(0, testBridge.m_changeCount);
        testIModelHubClientForBridges.m_expect.clear();
        }

    if (true)
        {
        // Run an update with a spatial data transform change
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = false;
        testBridge.m_expect.jobTransChanged = true;
        testBridge.m_changeCount = 0;
        testIModelHubClientForBridges.m_expect.push_back(true);//For project extents
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        args.push_back(transform_offset1_str.c_str());
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        args.pop_back();

        // *** TBD: Check that the elements moved
        EXPECT_EQ(2, testBridge.m_changeCount);
        testIModelHubClientForBridges.m_expect.clear();
        }

    if (true)
        {
        // Run an update with same transform => verify no changes
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = false;
        testBridge.m_expect.anyDeleted = false;
        testBridge.m_expect.jobTransChanged = false;
        testBridge.m_changeCount = 0;
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        args.push_back(transform_offset1_str.c_str());
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        args.pop_back();
        EXPECT_EQ(0, testBridge.m_changeCount);
        }

    if (true)
        {
        // Run an update with same transform passed via doc props => verify no changes
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = false;
        testBridge.m_expect.anyDeleted = false;
        testBridge.m_expect.jobTransChanged = false;
        testBridge.m_changeCount = 0;
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");

        iModelBridgeDocumentProperties fooDocProps;
        testRegistry._GetDocumentProperties(fooDocProps, BeFileName(L"Foo"));
        fooDocProps.m_spatialRootTransformJSON = transform_offset1.ToString();
        testRegistry.SetDocumentProperties(fooDocProps, BeFileName(L"Foo"));
        testRegistry.Save();
        
        //args.push_back(transform_offset1_str.c_str());
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        EXPECT_EQ(0, testBridge.m_changeCount);
        testIModelHubClientForBridges.m_expect.clear();
        }

    if (true)
        {
        // Run an update with a new transform passed via doc props => verify 2 changes
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = false;
        testBridge.m_expect.jobTransChanged = true;
        testBridge.m_changeCount = 0;
        testIModelHubClientForBridges.m_expect.push_back(true);//For project extents
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        iModelBridgeDocumentProperties fooDocProps;
        testRegistry._GetDocumentProperties(fooDocProps, BeFileName(L"Foo"));
        fooDocProps.m_spatialRootTransformJSON = transform_offset1_45.ToString();
        testRegistry.SetDocumentProperties(fooDocProps, BeFileName(L"Foo"));
        testRegistry.Save();
        //args.push_back(transform_offset1_str.c_str());
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        EXPECT_EQ(2, testBridge.m_changeCount);
        testIModelHubClientForBridges.m_expect.clear();
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, MixedFileTypeBridgeAssignmentTest)
    {
    auto testDir = getiModelBridgeTestsOutputDir(L"MixedFileTypeBridgeAssignmentTest");
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"test1Assignments.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();

    WString mstnBridgeRegSubKey(L"iModelBridgeForMstn");
    std::function<T_iModelBridge_getAffinity> mstnLamda = [=](BentleyApi::WCharP buffer,
                                                            const size_t bufferSize,
                                                            BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel,
                                                            BentleyApi::WCharCP affinityLibraryPath,
                                                            BentleyApi::WCharCP sourceFileName)
        {
        BeFileName srcFile(sourceFileName);
        if (srcFile.GetExtension().CompareToI(L"Dgn"))
            {
                
            affinityLevel = iModelBridgeAffinityLevel::Medium;
            wcsncpy(buffer, mstnBridgeRegSubKey.c_str(), mstnBridgeRegSubKey.length());
            }
        };

    testRegistry.AddBridge(mstnBridgeRegSubKey, mstnLamda);

        
    WString realDwgBridgeRegSubKey(L"RealDWG");
    std::function<T_iModelBridge_getAffinity> realDWGLamda = [=](BentleyApi::WCharP buffer,
                                                                const size_t bufferSize,
                                                                BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel,
                                                                BentleyApi::WCharCP affinityLibraryPath,
                                                                BentleyApi::WCharCP sourceFileName)
        {
        BeFileName srcFile(sourceFileName);
        if (srcFile.GetExtension().CompareToI(L"DWG"))
            {

            affinityLevel = iModelBridgeAffinityLevel::Medium;
            wcsncpy(buffer, realDwgBridgeRegSubKey.c_str(), realDwgBridgeRegSubKey.length());
            }
        };

    testRegistry.AddBridge(realDwgBridgeRegSubKey, realDWGLamda);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, DISABLED_TestMultipleRootsSameSubject_ToyTile) // disabled, because it uses ToyTile bridge, which is not expressed in a part dependency
    {
#define BRIDGE_REG_SUBKEY_TOY_TILE L"ToyTile"

    auto testDir = getiModelBridgeTestsOutputDir(L"TestMultipleRootsSameSubject_ToyTile");
	BeFileName bcName(testDir);
	bcName.AppendToPath(L"iModelBridgeTests_Test1.bim");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));
    
	BeFileName toyTileBridge_dll(L"d:\\bim0200dev\\out\\Winx64\\Product\\ToyTileBridge\\ToyTileBridge.dll");
	BeFileName toyTileBridge_assetsDir(toyTileBridge_dll.GetDirectoryName());
	toyTileBridge_assetsDir.AppendToPath(L"assets");
	BeFileName toyTileFile1(L"d:\\tmp\\toytile1.xml");
	BeFileName toyTileFile2(L"d:\\tmp\\toytile1.xml");

    bvector<WString> args;
    args.push_back(L"iModelBridgeTests.Test1");                                                 // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir.c_str()));
    args.push_back(L"--server-environment=Qa");
    args.push_back(L"--server-repository=iModelBridgeTests_Test1");                             // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-project-guid=iModelBridgeTests_Project");                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--fwk-create-repository-if-necessary");
    args.push_back(L"--fwk-revision-comment=\"comment in quotes\"");
    args.push_back(L"--server-user=username=username");                                         // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-password=\"password><!@\"");                                      // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg

	args.push_back(WPrintfString(L"--fwk-bridge-library=\"%ls\"", toyTileBridge_dll.c_str()));
    args.push_back(L"--fwk-bridge-regsubkey=" BRIDGE_REG_SUBKEY_TOY_TILE);
	args.push_back(WPrintfString(L"--fwk-bridgeAssetsDir=\"%ls\"", toyTileBridge_assetsDir.c_str()));

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubFwkClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);

    BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"test1Assignments.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    std::function<T_iModelBridge_getAffinity> lambda = [=](BentleyApi::WCharP buffer, const size_t bufferSize, BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel,
                                                          BentleyApi::WCharCP affinityLibraryPath, BentleyApi::WCharCP sourceFileName)
        {
        affinityLevel = iModelBridgeAffinityLevel::High;
        wcscpy(buffer, BRIDGE_REG_SUBKEY_TOY_TILE);
        };
    testRegistry.AddBridge(BRIDGE_REG_SUBKEY_TOY_TILE, lambda);

    iModelBridgeDocumentProperties fooDocProps(s_fooGuid, "wurn1", "durn1", "other1", "");
    iModelBridgeDocumentProperties barDocProps(s_barGuid, "wurn2", "durn2", "other2", "");
    testRegistry.SetDocumentProperties(fooDocProps, toyTileFile1);
    testRegistry.SetDocumentProperties(barDocProps, toyTileFile2);
    WString bridgeName;
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName,BeFileName(toyTileFile1),L"");
    testRegistry.SearchForBridgeToAssignToDocument(bridgeName,BeFileName(toyTileFile2),L"");
    
    testRegistry.AddRef(); // prevent ~iModelBridgeFwk from deleting this object.
    iModelBridgeFwk::SetRegistryForTesting(testRegistry);   // (takes ownership of pointer)

    args.push_back(L"--fwk-job-subject-name=TestMultipleRootsSameSubject_ToyTile"); // use the same job subject for all root files

    if (true)
        {
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
		args.push_back(WPrintfString(L"--fwk-input=%ls", toyTileFile1.c_str()));
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
		args.pop_back();
        }

	if (true)
		{
        ScopedDgnHost host;
		auto db = DgnDb::OpenDgnDb(nullptr, bcName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
		ASSERT_TRUE(db.IsValid());
		auto jobSubjects = getJobSubjects(*db);
		ASSERT_EQ(jobSubjects.size(), 1);
		ASSERT_STREQ(jobSubjects[0]->GetCode().GetValueUtf8CP(), "TestMultipleRootsSameSubject_ToyTile");
		}

	if (true)
        {
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
		args.push_back(WPrintfString(L"--fwk-input=%ls", toyTileFile2.c_str()));
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
		args.pop_back();
        }

	if (true)
		{
        ScopedDgnHost host;
		auto db = DgnDb::OpenDgnDb(nullptr, bcName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
		ASSERT_TRUE(db.IsValid());
		auto jobSubjects = getJobSubjects(*db);
		ASSERT_EQ(jobSubjects.size(), 1) << "Still just one job subject";
		ASSERT_STREQ(jobSubjects[0]->GetCode().GetValueUtf8CP(), "TestMultipleRootsSameSubject_ToyTile");
		}
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, LaunchDarklyQa)
    {
    bool flag;
    iModelBridgeLdClient& instance = iModelBridgeLdClient::GetInstance(WebServices::UrlProvider::Environment::Qa);
    instance.SetUserName("abeesh.basheer@bentley.com");
    instance.IsFeatureOn(flag, "allow-imodelhub-projectextents");
    ASSERT_EQ(true, flag);

    flag = false;
    ASSERT_EQ(SUCCESS, instance.RestartClient());
    instance.IsFeatureOn(flag, "allow-imodelhub-projectextents");
    ASSERT_EQ(true, flag);
    instance.Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, ECEFTransformTest)
    {
    auto bridgeRegSubKey = L"iModelBridgeTests_Test1_Bridge";

    auto testDir = getiModelBridgeTestsOutputDir(L"ECEFTransformTest");

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));
    
    // I have to create a file that I represent as the bridge "library", so that the fwk's argument validation logic will see that it exists.
    // The fwk won't try to load this file, since we will register a fake bridge.
    BeFileName fakeBridgeName(testDir);
    fakeBridgeName.AppendToPath(L"iModelBridgeTests-ECEFTransformTest");
    BeFile fakeBridgeFile;
    ASSERT_EQ(BeFileStatus::Success, fakeBridgeFile.Create(fakeBridgeName, true));
    fakeBridgeFile.Close();

    bvector<WString> args;
    args.push_back(L"iModelBridgeTests.ECEFTransformTest");                       // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir.c_str()));
    args.push_back(L"--server-environment=Qa");
    args.push_back(L"--server-repository=iModelBridgeTests_ECEFTransformTest");   // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-project-guid=iModelBridgeTests_Project");     // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--fwk-create-repository-if-necessary");        
    args.push_back(L"--server-user=username=username");                     // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-password=\"password><!@\"");                  // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-bridge-regsubkey=%ls", bridgeRegSubKey).c_str());  // must be consistent with testRegistry.m_bridgeRegSubKey
    args.push_back(WPrintfString(L"--fwk-bridge-library=\"%ls\"", fakeBridgeName.c_str())); // must refer to a path that exists! 
    BeFileName platformAssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
    args.push_back(WPrintfString(L"--fwk-bridgeAssetsDir=\"%ls\"", platformAssetsDir.c_str())); // must be a real assets dir! the platform's assets dir will serve just find as the test bridge's assets dir.

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubFwkClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);

    // Register the test bridge that fwk should run
    iModelBridgeTests_Test1_Bridge testBridge(testIModelHubClientForBridges);
    iModelBridgeFwk::SetBridgeForTesting(testBridge);

    
    BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"ECEFTransformTest.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    populateRegistryWithFooBar(testRegistry, bridgeRegSubKey);
    testRegistry.AddRef(); // prevent ~iModelBridgeFwk from deleting this object.
    iModelBridgeFwk::SetRegistryForTesting(testRegistry);   // (takes ownership of pointer)
    
    testBridge.m_expect.assignmentCheck = true;

    YawPitchRollAngles angles(AngleInDegrees::FromDegrees(30.0), AngleInDegrees::FromDegrees(40.0), AngleInDegrees::FromDegrees(50.0));    
    EcefLocation location(DPoint3d::From(1000.0, 2000.0, 3000.0), angles);

    Json::Value ecefJson = Json::Value(Json::ValueType::objectValue);
    ecefJson["ecef"] = location.ToJson();
    WPrintfString location_Str(L"--fwk-argsJson=\"%s\"", WString(ecefJson.ToString().c_str(), true).c_str());
    //--fwk-argsJson="{"ecef":{"orientation":{"pitch":40.0,"roll":50.0,"yaw":30.0},"origin":[1000.0,2000.0,3000.0]}}"
    if (true)
        {
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = false;
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = false;
        testBridge.m_expect.jobTransChanged = false;
        testIModelHubClientForBridges.m_expect.push_back(true);//For project extents
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        testIModelHubClientForBridges.m_expect.clear();
        }
    if (true)
        {
        // Run an update with a spatial data transform change
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = false;
        testBridge.m_expect.anyDeleted = false;
        testBridge.m_expect.jobTransChanged = false;
        testBridge.m_changeCount = 0;
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        args.push_back(location_Str.c_str());
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        args.pop_back();

        // *** TBD: Check that the elements moved
        EXPECT_EQ(0, testBridge.m_changeCount);
        testIModelHubClientForBridges.m_expect.clear();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static void DoProjectExtentsTest (bvector <double> &extents, WCharCP testName)
    {
    auto bridgeRegSubKey = L"iModelBridgeTests_Test1_Bridge";

    auto testDir = getiModelBridgeTestsOutputDir(testName);

    BeFileName bcName = testDir;
    bcName.AppendToPath(L"iModelBridgeTests_iModelProjectExtentsTest.bim");
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(testDir));

    // I have to create a file that I represent as the bridge "library", so that the fwk's argument validation logic will see that it exists.
    // The fwk won't try to load this file, since we will register a fake bridge.
    BeFileName fakeBridgeName(testDir);
    fakeBridgeName.AppendToPath(L"iModelBridgeTests-iModelProjectExtentsTest.bim");
    BeFile fakeBridgeFile;
    ASSERT_EQ(BeFileStatus::Success, fakeBridgeFile.Create(fakeBridgeName, true));
    fakeBridgeFile.Close();

    bvector<WString> args;
    args.push_back(L"iModelBridgeTests.iModelProjectExtentsTest");                       // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-staging-dir=\"%ls\"", testDir.c_str()));
    args.push_back(L"--server-environment=Qa");
    args.push_back(L"--server-repository=iModelBridgeTests_iModelProjectExtentsTest");   // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-project-guid=iModelBridgeTests_Project");     // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--fwk-create-repository-if-necessary");
    args.push_back(L"--server-user=imodelbridgetests@bentley.com");                     // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(L"--server-password=\"password><!@\"");                  // the value of this arg doesn't mean anything and is not checked by anything -- it is just a placeholder for a required arg
    args.push_back(WPrintfString(L"--fwk-bridge-regsubkey=%ls", bridgeRegSubKey).c_str());  // must be consistent with testRegistry.m_bridgeRegSubKey
    args.push_back(WPrintfString(L"--fwk-bridge-library=\"%ls\"", fakeBridgeName.c_str())); // must refer to a path that exists! 
    BeFileName platformAssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(platformAssetsDir);
    args.push_back(WPrintfString(L"--fwk-bridgeAssetsDir=\"%ls\"", platformAssetsDir.c_str())); // must be a real assets dir! the platform's assets dir will serve just find as the test bridge's assets dir.

    // Register our mock of the iModelHubClient API that fwk should use when trying to communicate with iModelHub
    TestIModelHubFwkClientForBridges testIModelHubClientForBridges(testDir);
    iModelBridgeFwk::SetIModelClientForBridgesForTesting(testIModelHubClientForBridges);

    // Register the test bridge that fwk should run
    iModelBridgeTests_Test1_Bridge testBridge(testIModelHubClientForBridges);
    iModelBridgeFwk::SetBridgeForTesting(testBridge);


    BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"iModelProjectExtentsTest.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    populateRegistryWithFooBar(testRegistry, bridgeRegSubKey);
    testRegistry.AddRef(); // prevent ~iModelBridgeFwk from deleting this object.
    iModelBridgeFwk::SetRegistryForTesting(testRegistry);   // (takes ownership of pointer)

    testBridge.m_expect.assignmentCheck = true;

    /* YawPitchRollAngles angles(AngleInDegrees::FromDegrees(30.0), AngleInDegrees::FromDegrees(40.0), AngleInDegrees::FromDegrees(50.0));
     EcefLocation location(DPoint3d::From(1000.0, 2000.0, 3000.0), angles);

     Json::Value ecefJson = Json::Value(Json::ValueType::objectValue);
     ecefJson["ecef"] = location.ToJson();
     WPrintfString location_Str(L"--fwk-argsJson=\"%s\"", WString(ecefJson.ToString().c_str(), true).c_str());*/
     //--fwk-argsJson="{"ecef":{"orientation":{"pitch":40.0,"roll":50.0,"yaw":30.0},"origin":[1000.0,2000.0,3000.0]}}"
    if (true)
        {
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = false;
        testBridge.m_expect.anyChanges = true;
        testBridge.m_expect.anyDeleted = false;
        testBridge.m_expect.jobTransChanged = false;
        testIModelHubClientForBridges.m_expect.push_back(true);//For project extents
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        testIModelHubClientForBridges.m_expect.clear();
        }
    
    if (true)
        {
        // Run an update with a spatial data transform change
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        //

        testIModelHubClientForBridges.GetIModelInfo()->SetExtent(extents);
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = false;
        testBridge.m_expect.anyDeleted = false;
        testBridge.m_expect.jobTransChanged = false;
        testIModelHubClientForBridges.m_expect.push_back(true);//For project extents
        testBridge.m_changeCount = 0;
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        args.push_back(L"--fwk-input=Foo");
        //args.push_back(location_Str.c_str());
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        args.pop_back();
        args.pop_back();

        // *** TBD: Check that the elements moved
        EXPECT_EQ(0, testBridge.m_changeCount);
        testIModelHubClientForBridges.m_expect.clear();
        }
        {
        ScopedDgnHost host;
        auto db = DgnDb::OpenDgnDb(nullptr, bcName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        ASSERT_TRUE(db.IsValid());
      
        AxisAlignedBox3d extents = db->GeoLocation().GetProjectExtents();
        bvector<BeInt64Id> elementOutliers;
        AxisAlignedBox3d rangeWithOutliers;
        AxisAlignedBox3d calculated = db->GeoLocation().ComputeProjectExtents(&rangeWithOutliers, &elementOutliers);
        calculated.IsContained(extents);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, iModelProjectExtentsTest)
    {
    bvector <double> extents = { 46.803981, -100.826828 , 46.843917, -100.764343 };
    DoProjectExtentsTest(extents, L"iModelProjectExtentsTest");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgeTests, iModelProjectExtentsTest_SouthAsia)
    {
    bvector <double> extents = { 18.938148519926713, 72.824610710144043 , 18.939833092473123, 72.826917409896851 };
    DoProjectExtentsTest(extents, L"iModelProjectExtentsTest_SouthAsia");
    }
