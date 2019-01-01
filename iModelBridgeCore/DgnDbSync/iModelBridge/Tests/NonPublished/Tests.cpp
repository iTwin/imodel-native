/*--------------------------------------------------------------------------------------+
|
|  $Source: iModelBridge/Tests/NonPublished/Tests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
#include <iModelBridge/Fwk/IModelClientForBridges.h>
#include <Bentley/BeFileName.h>
#include <iModelBridge/FakeRegistry.h>
#include <iModelBridge/TestiModelHubClientForBridges.h>
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

#define MAKE_ARGC_ARGV(argptrs, args)\
for (auto& arg: args)\
   argptrs.push_back(arg.c_str());\
int argc = (int)argptrs.size();\
wchar_t const** argv = argptrs.data();\

static Utf8CP s_fooGuid = "6640b375-a539-4e73-b3e1-2c0ceb912551";
static Utf8CP s_barGuid = "6640b375-a539-4e73-b3e1-2c0ceb912552";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<SubjectCPtr> getJobSubjects(DgnDbR db)
    {
	bvector<SubjectCPtr> subjects;
    auto childids = db.Elements().GetRootSubject()->QueryChildren();
    for (auto childid : childids)
        {
        auto subj = db.Elements().Get<Subject>(childid);
        if (subj.IsValid() && JobSubjectUtils::IsJobSubject(*subj))
            subjects.push_back(subj);
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

        DgnV8FileProvenance::CreateTable(*db);
        DgnV8ModelProvenance::CreateTable(*db);
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
    BentleyStatus _DetectDeletedDocuments() override { return BSISUCCESS;}

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

    bvector<iModelBridgeSyncInfoFile::ROWID> scopes = {scope1, scope2};

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
        // It will get cleaned up when we call DeleteElementsNotSeenInScope later on.
        ++expected_counts.first;
        ASSERT_EQ(expected_counts, countItemsInSyncInfo(syncInfo));
        ASSERT_EQ(expected_counts.first-2, countElementsOfClass(iModelBridgeTests::GetGenericPhysicalObjectClassId(db), db));

        //  Now tell syncinfo to garbage-collect the elements that were abandoned.
        changeDetector._DeleteElementsNotSeenInScopes(scopes);

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
        changeDetector._DeleteElementsNotSeenInScopes(scopes);    // s/ not do anything.

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
        {  }

        virtual DgnRevisionPtr CaptureChangeSet(DgnDbP db, Utf8CP comment) override;
    };
END_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsistruct                                                   Sam.Wilson   04/17
//=======================================================================================
struct iModelBridgeTests_Test1_Bridge : iModelBridgeWithSyncInfoBase
{
    TestSourceItemWithId m_foo_i0;
    TestSourceItemWithId m_foo_i1;
    TestSourceItemWithId m_bar_i0;
    TestSourceItemWithId m_bar_i1;
    TestIModelHubFwkClientForBridges& m_testIModelHubClientForBridges;
    iModelBridgeSyncInfoFile::ROWID m_docScopeId;
    bool m_jobTransChanged = false;
    int m_changeCount = 0;

    struct
        {
        bool findJobSubject = false;
        bool anyChanges = false;
        bool anyDeleted = false;
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
    void _DeleteSyncInfo() override {iModelBridgeSyncInfoFile::DeleteSyncInfoFileFor(_GetParams().GetBriefcaseName());}

    void _OnDocumentDeleted(Utf8StringCR docId, iModelBridgeSyncInfoFile::ROWID docrid) override
        {
        ASSERT_TRUE(std::find(m_expect.docsDeleted.begin(), m_expect.docsDeleted.end(), docId) != m_expect.docsDeleted.end());

        for (auto rec : m_syncInfo.MakeIteratorByScope(docrid))
            {
            ASSERT_TRUE(m_expect.anyDeleted);
            if (!rec.GetDgnElementId().IsValid())
                continue; // not every record in syncinfo is an element
            auto el = GetDgnDbR().Elements().GetElement(rec.GetDgnElementId());
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

        // register the document. This then becomes the scope for all of my items.
        iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(*GetSyncInfo().GetChangeDetectorFor(*this), _GetParams().GetInputFileName());

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

        if (!GetDgnDbR().TableExists(DGN_TABLE_ProvenanceFile))
            DgnV8FileProvenance::CreateTable(GetDgnDbR());
        if (!GetDgnDbR().TableExists(DGN_TABLE_ProvenanceModel))
            DgnV8ModelProvenance::CreateTable(GetDgnDbR());
        return subjectObj->InsertT<Subject>();
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
        m_foo_i0("0", "foo i0 - initial"),
        m_foo_i1("1", "foo i1 - initial"),
        m_bar_i0("0", "bar i0 - initial"),
        m_bar_i1("1", "bar i1 - initial"),
        m_testIModelHubClientForBridges(tc)
        {}
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
    bool expectedResult = m_expect.front();
    m_expect.pop_front();

    BeAssert(expectedResult == anyTxnsInFile(*db));

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
    Utf8CP itemKind = "";   // we don't use kinds in this test

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

    if (m_expect.assignmentCheck)
        {
        ASSERT_TRUE(IsFileAssignedToBridge(_GetParams().GetInputFileName()));
        }

    iModelBridgeSyncInfoFile::ChangeDetectorPtr changeDetector = GetSyncInfo().GetChangeDetectorFor(*this);

    iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(*changeDetector, _GetParams().GetInputFileName());
    m_docScopeId = docLink.m_syncInfoRecord.GetROWID();

    Transform _newTrans, _oldTrans;
    m_jobTransChanged = DetectSpatialDataTransformChange(_newTrans, _oldTrans, *changeDetector, m_docScopeId, "JT", "JT");
    ASSERT_EQ(m_expect.jobTransChanged, m_jobTransChanged);

    // Convert the "items" in my (non-existant) source file.
    if (_GetParams().GetInputFileName() == L"Foo")
        {
        ConvertItem(m_foo_i0, *changeDetector);
        ConvertItem(m_foo_i1, *changeDetector);
        }
    else
        {
        ConvertItem(m_bar_i0, *changeDetector);
        ConvertItem(m_bar_i1, *changeDetector);
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
    args.push_back(L"--fwk-storeElementIdsInBIM");
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
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        testIModelHubClientForBridges.m_expect.clear();
        }

    if (true)
        {
        // Modify an item 
        testBridge.m_foo_i0.m_content = "changed";

        // and run an update
        // This time, we expect to find the repo and briefcase already there.
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testIModelHubClientForBridges.m_expect.push_back(true);// This will be set since we import the aspect schema.
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
    //args.push_back(L"--fwk-storeElementIdsInBIM");
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

    if (true)
        {
        // now pretend that the document called "bar" was deleted.
        testRegistry.RemoveFileAssignment(BeFileName(L"Bar"));
        testIModelHubClientForBridges.m_expect.push_back(false);// Clear this flag at the outset. It is set by the test bridge as it runs.
        testBridge.m_expect.findJobSubject = true;
        testBridge.m_expect.anyChanges = false;
        testBridge.m_expect.anyDeleted = true;
        testBridge.m_expect.docsDeleted.push_back(s_barGuid);
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        // args.push_back(L"--fwk-input=Foo");      specify no input to simulate the edge case
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        // args.pop_back();
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
    args.push_back(L"--fwk-storeElementIdsInBIM");
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
