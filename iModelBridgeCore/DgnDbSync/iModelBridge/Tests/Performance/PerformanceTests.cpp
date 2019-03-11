/*--------------------------------------------------------------------------------------+
|
|  $Source: iModelBridge/Tests/Performance/PerformanceTests.cpp $
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
#include <iModelBridge/IModelClientForBridges.h>
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


//=======================================================================================
// @bsistruct                                                   Sam.Wilson   04/17
//=======================================================================================
struct iModelBridgePerformanceTests : ::testing::Test
{
    static BeFileName GetOutputDir()
        {
        BeFileName bcDir;
        BeTest::GetHost().GetOutputRoot(bcDir);
        bcDir.AppendToPath(L"iModelBridgePerformanceTests");
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
        createProjectParams.SetRootSubjectName("iModelBridgePerformanceTests");

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
    int m_id;
    int m_content;
    iModelBridgeSyncInfoFile::ROWID m_scope;
    Utf8String m_kind;
    TestSourceItemWithId(int id, int content, iModelBridgeSyncInfoFile::ROWID scope, Utf8StringCR kind)
        : m_id(id),m_content(content),m_scope(scope),m_kind(kind)
    {}
    Utf8String _GetId() override  {return Utf8PrintfString("%d",m_id);}
    double _GetLastModifiedTime() override {return 0.0;}
    Utf8String _GetHash() override
        {
        SHA1 sha1;
        sha1(Utf8PrintfString("%d", m_content));
        return sha1.GetHashString();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName getiModelBridgeTestsOutputDir(WCharCP subdir)
    {
    BeFileName testDir;
    BeTest::GetHost().GetOutputRoot(testDir);
    testDir.AppendToPath(L"iModelBridgePerformanceTests");
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
    TestIModelHubFwkClientForBridges(BeFileNameCR testWorkDir) 
        : TestIModelHubClientForBridges(testWorkDir)
        {  }

        virtual DgnRevisionPtr CaptureChangeSet(DgnDbP db, Utf8CP comment) override;
    };
END_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsistruct                                                   Sam.Wilson   04/17
//=======================================================================================
struct iModelBridgeTests_PerfTest_Bridge : iModelBridgeWithSyncInfoBase
{
    TestIModelHubFwkClientForBridges& m_testIModelHubClientForBridges;
    iModelBridgeSyncInfoFile::ROWID m_docScopeId;
    bool m_jobTransChanged = false;
    int m_changeCount = 0;

    
    int m_numItems;
    int m_numScopes;
    std::vector< iModelBridgeSyncInfoFile::ROWID> m_scopes;
    std::function <TestSourceItemWithId (int )> m_generator;

    Utf8String ComputeJobSubjectCodeValue()
        {
        // note that the jobs are root specific and so the job subject's code must include the input filename
        Utf8String jobCode("iModelBridgeTests_PerfTest_Bridge - ");
        jobCode.append(Utf8String(_GetParams().GetInputFileName()).c_str());
        return jobCode;
        }

    WString _SupplySqlangRelPath() override {return L"sqlang/DgnPlatform_en.sqlang.db3";}
    BentleyStatus _Initialize(int argc, WCharCP argv[]) override {return BSISUCCESS;}

    void _OnDocumentDeleted(Utf8StringCR docId, iModelBridgeSyncInfoFile::ROWID docrid) override
        {
        }

    SubjectCPtr _FindJob() override
        {
        DgnCode jobCode = Subject::CreateCode(*GetDgnDbR().Elements().GetRootSubject(), ComputeJobSubjectCodeValue().c_str());
        auto jobId = GetDgnDbR().Elements().QueryElementIdByCode(jobCode);
        return GetDgnDbR().Elements().Get<Subject>(jobId);
        }

    SubjectCPtr _InitializeJob() override
        {
        // Set up the model and category that my superclass's DoTest method uses
        DgnCode partitionCode = PhysicalPartition::CreateCode(*GetDgnDbR().Elements().GetRootSubject(), "PhysicalModel");
        if (!GetDgnDbR().Elements().QueryElementIdByCode(partitionCode).IsValid())
            {
            DgnDbTestUtils::InsertPhysicalModel(GetDgnDbR(), "PhysicalModel");
            DgnDbTestUtils::InsertSpatialCategory(GetDgnDbR(), "SpatialCategory");
            }

        auto subjectObj = Subject::Create(*GetDgnDbR().Elements().GetRootSubject(), ComputeJobSubjectCodeValue().c_str());
        JobSubjectUtils::InitializeProperties(*subjectObj, _GetParams().GetBridgeRegSubKeyUtf8());
        
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

    DgnElementId ConvertItem(TestSourceItemWithId& item, iModelBridgeSyncInfoFile::ChangeDetector&);

    iModelBridgeTests_PerfTest_Bridge(TestIModelHubFwkClientForBridges& tc)
        :
        iModelBridgeWithSyncInfoBase(),
        m_testIModelHubClientForBridges(tc)
        {
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
    
    // printf("*** %s : %d\n", comment, anyTxnsInFile(*db));

    if (!anyTxnsInFile(*db))
        return nullptr;

    DgnRevisionPtr changeSet = db->Revisions().StartCreateRevision();

    if (!changeSet.IsValid())
        {
        db->Revisions().FinishCreateRevision();
        return changeSet;
        }

    if (comment)
        changeSet->SetSummary(comment);

    BeAssert(changeSet.IsValid());
    auto revStatus = db->Revisions().FinishCreateRevision();
    BeAssert(Dgn::RevisionStatus::Success == revStatus);
    auto dbStatus = db->SaveChanges();
    BeAssert(BE_SQLITE_OK == dbStatus);

    return changeSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId iModelBridgeTests_PerfTest_Bridge::ConvertItem(TestSourceItemWithId& item, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector)
    {
    auto change = changeDetector._DetectChange(item.m_scope, item.m_kind.c_str(), item, nullptr, m_jobTransChanged);
    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == change.GetChangeType())
        {
        changeDetector._OnElementSeen(change.GetSyncInfoRecord().GetDgnElementId());
        return change.GetSyncInfoRecord().GetDgnElementId();
        }
    
    ++m_changeCount;
    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = iModelBridgePerformanceTests::CreateGenericPhysicalObject(*m_db);
    DgnElementTransformer::ApplyTransformTo(*results.m_element, GetSpatialDataTransform());
    auto status = changeDetector._UpdateBimAndSyncInfo(results, change);
    BeAssert(BE_SQLITE_OK == status);
    return results.m_element->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeTests_PerfTest_Bridge::DoConvertToBim(SubjectCR jobSubject)
    {
    // (Note: superclass iModelBridgeWithSyncInfoBase::_OnConvertToBim has already attached my syncinfo file to the bim.)

    
    iModelBridgeSyncInfoFile::ChangeDetectorPtr changeDetector = GetSyncInfo().GetChangeDetectorFor(*this);

    iModelBridgeSyncInfoFile::ConversionResults docLink = RecordDocument(*changeDetector, _GetParams().GetInputFileName(), nullptr,
        "DocumentWithBeGuid", iModelBridgeSyncInfoFile::ROWID(jobSubject.GetElementId().GetValue()));

    m_docScopeId = docLink.m_element->GetElementId().GetValue();

    Transform _newTrans, _oldTrans;
    m_jobTransChanged = DetectSpatialDataTransformChange(_newTrans, _oldTrans, *changeDetector, m_docScopeId, "JT", "JT");
    //Create scopes
    m_scopes.clear();
    for (int index = 0; index < m_numScopes; ++index)
        {
        TestSourceItemWithId scopeItem(index, index, m_docScopeId, "Scope");
        m_scopes.push_back(iModelBridgeSyncInfoFile::ROWID (ConvertItem(scopeItem, *changeDetector).GetValue()));
        }
    
    // Convert the "items" in my (non-existant) source file.
    for (int index =0; index <m_numItems; ++ index)
        {
        TestSourceItemWithId item = m_generator(index);
        ConvertItem(item, *changeDetector);
        }

    //  Garbage-collect the elements that were abandoned.
    changeDetector->DeleteElementsNotSeenInScope(m_docScopeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelBridgePerformanceTests, PerformanceTest)
    {
    auto bridgeRegSubKey = L"iModelBridgeTests_PerfTest_Bridge";

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
    iModelBridgeTests_PerfTest_Bridge testBridge(testIModelHubClientForBridges);
    iModelBridgeFwk::SetBridgeForTesting(testBridge);

    BeFileName assignDbName(testDir);
    assignDbName.AppendToPath(L"test1Assignments.db");
    FakeRegistry testRegistry(testDir, assignDbName);
    testRegistry.WriteAssignments();
    populateRegistryWithFooBar(testRegistry, bridgeRegSubKey);
    
    testRegistry.AddRef(); // prevent ~iModelBridgeFwk from deleting this object.
    iModelBridgeFwk::SetRegistryForTesting(testRegistry);   // (takes ownership of pointer)

    static const int numFiles = 120;
    static const int numElementPerFile = 10000;

    testBridge.m_numScopes = numFiles;
    testBridge.m_numItems = 0;

    if (true)
        {
        // Ask the framework to run our test bridge to do the initial conversion and create the repo
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }
    
    testBridge.m_numScopes = numFiles;
    testBridge.m_numItems = numFiles * numElementPerFile;
    if (true)
        {
        // Add items
        //testBridge.m_foo_items[0].m_content = "changed";
        testBridge.m_generator = [&](int index){
        int scopeId = index % numElementPerFile;
        TestSourceItemWithId item(index, index, testBridge.m_scopes[scopeId], "Item");
        return item;
        };
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }

    if (true)
        {
        // and run an update
        testBridge.m_generator = [&](int index) {
            int scopeId = index % numElementPerFile;
            TestSourceItemWithId item(index, 2 * index, testBridge.m_scopes[scopeId], "Item");
            return item;
        };
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }
    if (true)
        {
        // Run an update with no changes
        testBridge.m_generator = [&](int index){
        int scopeId = index % numElementPerFile;
        TestSourceItemWithId item(index, 2 * index, testBridge.m_scopes[scopeId], "Item");
        return item;
        };
        iModelBridgeFwk fwk;
        bvector<WCharCP> argptrs;
        MAKE_ARGC_ARGV(argptrs, args);
        ASSERT_EQ(BentleyApi::BSISUCCESS, fwk.ParseCommandLine(argc, argv));
        ASSERT_EQ(0, fwk.Run(argc, argv));
        }
    }
