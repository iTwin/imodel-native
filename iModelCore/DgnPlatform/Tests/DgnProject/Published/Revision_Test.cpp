/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"
#include <DgnPlatform/DgnChangeSummary.h>
#include <DgnPlatform/GenericDomain.h>
#include <array>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>

// #define DEBUG_REVISION_TEST_MANUAL 1
#ifdef DEBUG_REVISION_TEST_MANUAL 
#include <windows.h>
#endif

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_EC

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"DgnCore"))

// Turn this on for debugging.
// #define DUMP_REVISION 1
// #define DUMP_CODES

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct RevisionTestFixture : ChangeTestFixture
{
DEFINE_T_SUPER(ChangeTestFixture)
protected:
    int m_z = 0;
    WCharCP m_copyTestFileName = L"RevisionTestCopy.ibim";

    void InsertFloor(int xmax, int ymax);
    void ModifyElement(DgnElementId elementId);

    DgnRevisionPtr CreateRevision();
    void DumpRevision(DgnRevisionCR revision, Utf8CP summary = nullptr);

    void BackupTestFile();
    void RestoreTestFile(Db::OpenMode openMode = Db::OpenMode::ReadWrite);

    void ExtractCodesFromRevision(DgnCodeSet& assigned, DgnCodeSet& discarded);
    void ProcessSchemaRevision(DgnRevisionCR revision, RevisionProcessOption revisionProcessOption);
    void MergeSchemaRevision(DgnRevisionCR revision) { ProcessSchemaRevision(revision, RevisionProcessOption::Merge); }
    void ReverseSchemaRevision(DgnRevisionCR revision) { ProcessSchemaRevision(revision, RevisionProcessOption::Reverse); }
    void ReinstateSchemaRevision(DgnRevisionCR revision) { ProcessSchemaRevision(revision, RevisionProcessOption::Reinstate); }

    static Utf8String CodeToString(DgnCodeCR code) { return Utf8PrintfString("%s:%s\n", code.GetScopeString().c_str(), code.GetValueUtf8CP()); }
    static void ExpectCode(DgnCodeCR code, DgnCodeSet const& codes) { EXPECT_FALSE(codes.end() == codes.find(code)) << CodeToString(code).c_str(); }
    static void ExpectCodes(DgnCodeSet const& exp, DgnCodeSet const& actual)
        {
        EXPECT_EQ(exp.size(), actual.size());
        for (auto const& code : exp)
            ExpectCode(code, actual);
        }

    static void DumpCode(DgnCodeCR code) { printf("    %s\n", CodeToString(code).c_str()); }
    static void DumpCodes(DgnCodeSet const& codes, Utf8StringCR msg="Codes:")
        {
#ifdef DUMP_CODES
        printf("%s\n", msg.c_str());
        for (auto const& code : codes)
            DumpCode(code);
#endif
        }

    AnnotationTextStyleCPtr CreateTextStyle(Utf8CP name)
        {
        AnnotationTextStyle style(*m_db);
        style.SetName(name);
        auto pStyle = style.Insert();
        EXPECT_TRUE(pStyle.IsValid());
        return pStyle;
        }
    AnnotationTextStyleCPtr RenameTextStyle(AnnotationTextStyleCR style, Utf8CP newName)
        {
        auto pStyle = style.CreateCopy();
        pStyle->SetName(newName);
        auto cpStyle = pStyle->Update();
        EXPECT_TRUE(cpStyle.IsValid());
        return cpStyle;
        }

    DgnElementCPtr InsertPhysicalElementByCode(DgnCodeCR code)
        {
        DgnClassId classId = m_db->Domains().GetClassId(generic_ElementHandler::PhysicalObject::GetHandler());
        GenericPhysicalObject elem(GenericPhysicalObject::CreateParams(*m_db, m_defaultModel->GetModelId(), classId, m_defaultCategoryId, Placement3d(), code, nullptr, DgnElementId()));
        return elem.Insert();
        }

    DgnElementCPtr RenameElement(DgnElementCR el, DgnCodeCR code)
        {
        auto pEl = el.CopyForEdit();
        EXPECT_EQ(DgnDbStatus::Success, pEl->SetCode(code));
        auto cpEl = pEl->Update();
        EXPECT_TRUE(cpEl.IsValid());
        return cpEl;
        }
public:
    static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;
    static void SetUpTestCase();

    RevisionTestFixture() {}
};

DgnPlatformSeedManager::SeedDbInfo RevisionTestFixture::s_seedFileInfo;
//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2016
//---------------------------------------------------------------------------------------
void RevisionTestFixture::SetUpTestCase()
    {
    //Start from parent fixture's seed Db
    ScopedDgnHost tempHost;

    //  Request a root seed file.
    DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(ChangeTestFixture::s_seedFileInfo.id, DgnPlatformSeedManager::SeedDbOptions(true, true));

    //  The group's seed file is essentially the same as the root seed file, but with a different relative path.
    //  Note that we must put our group seed file in a group-specific sub-directory
    RevisionTestFixture::s_seedFileInfo = rootSeedInfo;
    RevisionTestFixture::s_seedFileInfo.fileName.SetName(L"RevisionTestFixture/RevisionTestFixture.bim");

    DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(rootSeedInfo.fileName, RevisionTestFixture::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());
    TestDataManager::MustBeBriefcase(db, Db::OpenMode::ReadWrite);

    m_defaultCodeSpecId = DgnDbTestUtils::InsertCodeSpec(*db, "TestCodeSpec");
    ASSERT_TRUE(m_defaultCodeSpecId.IsValid());

    db->SaveChanges();
    // Create a dummy revision to purge transaction table for the test
    DgnRevisionPtr rev = db->Revisions().StartCreateRevision();
    BeAssert(rev.IsValid());
    db->Revisions().FinishCreateRevision();

    db->SaveChanges();

    PhysicalModelPtr model = db->Models().Get<PhysicalModel>(DgnDbTestUtils::QueryFirstGeometricModelId(*db));
    ASSERT_TRUE(model.IsValid());
    int z = 1;
    for (int x = 0; x < 1; x++)
        for (int y = 0; y < 1; y++)
            InsertPhysicalElement(*db, *model, DgnDbTestUtils::GetFirstSpatialCategoryId(*db) , x, y, z);

    CreateDefaultView(*db);
    DgnDbTestUtils::UpdateProjectExtents(*db);
    db->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void RevisionTestFixture::InsertFloor(int xmax, int ymax)
    {
    int z = 1;
    for (int x = 0; x < xmax; x++)
        for (int y = 0; y < ymax; y++)
            RevisionTestFixture::InsertPhysicalElement(*m_db, *m_defaultModel, m_defaultCategoryId, x, y, z);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void RevisionTestFixture::DumpRevision(DgnRevisionCR revision, Utf8CP summary)
    {
#ifdef DUMP_REVISION
    LOG.infov("---------------------------------------------------------");
    if (summary != nullptr)
        LOG.infov(summary);
    revision.Dump(*m_db);
    LOG.infov("---------------------------------------------------------");
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void RevisionTestFixture::ModifyElement(DgnElementId elementId)
    {
    RefCountedPtr<PhysicalElement> testElement = m_db->Elements().GetForEdit<PhysicalElement>(elementId);
    ASSERT_TRUE(testElement.IsValid());

    Placement3d newPlacement = testElement->GetPlacement();
    newPlacement.GetOriginR().x += 1.0;

    testElement->SetPlacement(newPlacement);

    DgnDbStatus dbStatus;
    testElement->Update(&dbStatus);
    ASSERT_TRUE(dbStatus == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionTestFixture::CreateRevision()
    {
    DgnRevisionPtr revision = m_db->Revisions().StartCreateRevision();
    if (!revision.IsValid())
        return nullptr;

    RevisionStatus status = m_db->Revisions().FinishCreateRevision();
    if (RevisionStatus::Success != status)
        {
        BeAssert(false);
        return nullptr;
        }

    return revision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2017
//---------------------------------------------------------------------------------------
void RevisionTestFixture::ProcessSchemaRevision(DgnRevisionCR revision, RevisionProcessOption revisionProcessOption)
    {
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    CloseDgnDb();
    
    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(revision, revisionProcessOption));
    m_db = DgnDb::OpenDgnDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    m_defaultCodeSpec = m_db->CodeSpecs().GetCodeSpec(m_defaultCodeSpecId);
    ASSERT_TRUE(m_defaultCodeSpec.IsValid());

    m_defaultModel = m_db->Models().Get<PhysicalModel>(m_defaultModelId);
    ASSERT_TRUE(m_defaultModel.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void RevisionTestFixture::BackupTestFile()
    {
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    CloseDgnDb();
    BeFileName originalFile = fileName;
    BeFileName copyFile(originalFile.GetDirectoryName()); //DgnDbTestDgnManager::GetOutputFilePath(m_copyTestFileName);
    copyFile.AppendToPath(m_copyTestFileName);
    //BeFileName copyFile = DgnDbTestDgnManager::GetOutputFilePath(m_copyTestFileName);

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(originalFile.c_str(), copyFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenDgnDb(fileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void RevisionTestFixture::RestoreTestFile(Db::OpenMode openMode /*= Db::OpenMode::ReadWrite*/)
    {
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    CloseDgnDb();
    BeFileName originalFile = fileName;
    BeFileName copyFile(originalFile.GetDirectoryName()); //DgnDbTestDgnManager::GetOutputFilePath(m_copyTestFileName);
    copyFile.AppendToPath(m_copyTestFileName);

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(copyFile.c_str(), originalFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenDgnDb(fileName, openMode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, Workflow)
    {
    // Setup a model with a few elements
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"Workflow.bim");
    m_db->SaveChanges("Created Initial Model");

    // Create an initial revision
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());

    Utf8String initialParentRevId = m_db->Revisions().GetParentRevisionId();

    // Create and save multiple revisions
    BackupTestFile();
    bvector<DgnRevisionPtr> revisions;
    int dimension = 5;
    int numRevisions = 5;
    for (int revNum = 0; revNum < numRevisions; revNum++)
        {
        InsertFloor(dimension, dimension);
        m_db->SaveChanges("Inserted floor");

        DgnRevisionPtr revision = CreateRevision();
        ASSERT_TRUE(revision.IsValid());
        ASSERT_FALSE(revision->ContainsSchemaChanges(*m_db));

        revisions.push_back(revision);
        }
    RestoreTestFile();

    // Dump all revisions
    for (DgnRevisionPtr const& rev : revisions)
        DumpRevision(*rev);

    // Merge all the saved revisions
    for (DgnRevisionPtr const& rev : revisions)
        {
        RevisionStatus status = m_db->Revisions().MergeRevision(*rev);
        ASSERT_TRUE(status == RevisionStatus::Success);
        }

    // Check the updated revision id
    Utf8String mergedParentRevId = m_db->Revisions().GetParentRevisionId();
    ASSERT_TRUE(mergedParentRevId != initialParentRevId);

    // Abandon changes, and test that the parent revision and elements do not change
    m_db->AbandonChanges();

    Utf8String newParentRevId = m_db->Revisions().GetParentRevisionId();
    ASSERT_TRUE(newParentRevId == mergedParentRevId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MoreWorkflow)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"MoreWorkflow.bim");
    m_db->SaveChanges("Created Initial Model");
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());

    // Create Revision 1 inserting an element into the test model
    BackupTestFile();
    DgnElementId elementId = RevisionTestFixture::InsertPhysicalElement(*m_db, *m_defaultModel, m_defaultCategoryId, 2, 2, 2);
    ASSERT_TRUE(elementId.IsValid());
    m_db->SaveChanges("Inserted an element");

    DgnRevisionPtr revision1 = CreateRevision();
    ASSERT_TRUE(revision1.IsValid());
    ASSERT_FALSE(revision1->ContainsSchemaChanges(*m_db));

    // Create Revision 2 after deleting the same element
    DgnElementCPtr el = m_db->Elements().Get<DgnElement>(elementId);
    ASSERT_TRUE(el.IsValid());
    DgnDbStatus status = m_db->Elements().Delete(*el);
    ASSERT_TRUE(status == DgnDbStatus::Success);
    el = nullptr;
    m_db->SaveChanges("Deleted same element");

    DgnRevisionPtr revision2 = CreateRevision();
    ASSERT_TRUE(revision2.IsValid());
    ASSERT_FALSE(revision2->ContainsSchemaChanges(*m_db));

    // Create Revision 3 deleting the test model (the API causes Elements to get deleted)
    RestoreTestFile();
    ASSERT_TRUE(m_defaultModel.IsValid());
    status = m_defaultModel->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);
    m_defaultModel = nullptr;
    m_db->SaveChanges("Deleted model and contained elements");

    DgnRevisionPtr revision3 = CreateRevision();
    ASSERT_TRUE(revision3.IsValid());
    ASSERT_FALSE(revision3->ContainsSchemaChanges(*m_db));

    RevisionStatus revStatus;

    // Merge Rev1 first
    RestoreTestFile();
    ASSERT_TRUE(m_defaultModel.IsValid());
    revStatus = m_db->Revisions().MergeRevision(*revision1);
    ASSERT_TRUE(revStatus == RevisionStatus::Success);

    // Merge Rev2 next
    revStatus = m_db->Revisions().MergeRevision(*revision2);
    ASSERT_TRUE(revStatus == RevisionStatus::Success);

    // Merge Rev3 next - should fail since the parent does not match
    BeTest::SetFailOnAssert(false);
    revStatus = m_db->Revisions().MergeRevision(*revision3);
    BeTest::SetFailOnAssert(true);
    ASSERT_TRUE(revStatus == RevisionStatus::ParentMismatch);

    // Merge Rev3 first
    RestoreTestFile();
    ASSERT_TRUE(m_defaultModel.IsValid());
    revStatus = m_db->Revisions().MergeRevision(*revision3);
    ASSERT_TRUE(revStatus == RevisionStatus::Success);
    
    // Delete model and Merge Rev1 - should fail since the model does not exist
    RestoreTestFile();
    status = m_defaultModel->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);
    m_defaultModel = nullptr;
    m_db->SaveChanges("Deleted model and contained elements");

    BeTest::SetFailOnAssert(false);
    revStatus = m_db->Revisions().MergeRevision(*revision1);
    BeTest::SetFailOnAssert(true);
    ASSERT_TRUE(revStatus == RevisionStatus::ApplyError);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MergeToReadonlyBriefcase)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"ReadonlyBriefcase.bim");
    m_db->SaveChanges();
    DgnRevisionPtr initialRevision = CreateRevision(); // Clears Txn table
    BackupTestFile();

    // Create some revision
    DgnElementId elementId = RevisionTestFixture::InsertPhysicalElement(*m_db, *m_defaultModel, m_defaultCategoryId, 2, 2, 2);
    ASSERT_TRUE(elementId.IsValid());
    m_db->SaveChanges("Inserted an element");
    DgnRevisionPtr revision1 = CreateRevision();
    ASSERT_TRUE(revision1.IsValid());

    // Restore the master file again, and open in Readonly mode
    RestoreTestFile(Db::OpenMode::Readonly);
   
    // Merge revision that was previously created to create a checkpoint file
    BeTest::SetFailOnAssert(false);
    RevisionStatus revStatus = m_db->Revisions().MergeRevision(*revision1);
    ASSERT_TRUE(revStatus == RevisionStatus::CannotMergeIntoReadonly);
    BeTest::SetFailOnAssert(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MergeToMaster)
    {
    // Setup master file
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"Master.bim");
    m_db->SaveChanges();
    DgnRevisionPtr initialRevision = CreateRevision(); // Clears Txn table
    m_db->SetAsMaster();
    m_db->SaveChanges("Setup master seed file");
    BackupTestFile();

    // Create some revision
    m_db->SetAsBriefcase(BeBriefcaseId(BeBriefcaseId::Standalone()));
    m_db->SaveChanges("Created briefcase");
    DgnElementId elementId = RevisionTestFixture::InsertPhysicalElement(*m_db, *m_defaultModel, m_defaultCategoryId, 2, 2, 2);
    ASSERT_TRUE(elementId.IsValid());
    m_db->SaveChanges("Inserted an element");
    DgnRevisionPtr revision1 = CreateRevision();
    ASSERT_TRUE(revision1.IsValid());

    // Restore the master file again
    RestoreTestFile();

    // Merge revision that was previously created to create a checkpoint file
    BeTest::SetFailOnAssert(false);
    RevisionStatus revStatus = m_db->Revisions().MergeRevision(*revision1);
    ASSERT_TRUE(revStatus == RevisionStatus::CannotMergeIntoMaster);
    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionTestFixture::ExtractCodesFromRevision(DgnCodeSet& assigned, DgnCodeSet& discarded)
    {
    m_db->SaveChanges();
    DgnRevisionPtr rev = m_db->Revisions().StartCreateRevision();
    BeAssert(rev.IsValid());

    rev->ExtractCodes(assigned, discarded, *m_db);

    m_db->Revisions().FinishCreateRevision();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RevisionTestFixture, Codes)
    {
    // Creating the DgnDb allocates some codes (category, model, view...)
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"Codes.bim");

    DgnDbR db = *m_db;

    SpatialCategoryCPtr defaultCat = SpatialCategory::Get(db, m_defaultCategoryId);
    DgnSubCategory subCat(DgnSubCategory::CreateParams(db, m_defaultCategoryId, "MySubCategory", DgnSubCategory::Appearance()));
    auto cpSubCat = subCat.Insert();
    EXPECT_TRUE(cpSubCat.IsValid());

    DgnModelPtr defaultModel = m_defaultModel;
    ASSERT_TRUE(defaultCat.IsValid());
    ASSERT_TRUE(defaultModel.IsValid());

    // Check that the new codes are all reported
    DgnCodeSet createdCodes, discardedCodes;
    ExtractCodesFromRevision(createdCodes, discardedCodes);

    DgnCodeSet expectedCodes;
    ExpectCodes(expectedCodes, discardedCodes);
    
    expectedCodes.insert(subCat.GetCode());
    expectedCodes.insert(ViewDefinition::CreateCode(db.GetDictionaryModel(), "Default"));
    ExpectCodes(expectedCodes, createdCodes);

    // Create some new elements with codes, and delete one with a code
    auto cpStyleA = CreateTextStyle("A"),
         cpStyleB = CreateTextStyle("B");

    auto subCatCode = cpSubCat->GetCode();
    EXPECT_EQ(DgnDbStatus::Success, cpSubCat->Delete());

    ExtractCodesFromRevision(createdCodes, discardedCodes);
    expectedCodes.clear();
    expectedCodes.insert(subCatCode);
    ExpectCodes(expectedCodes, discardedCodes);

    expectedCodes.clear();
    expectedCodes.insert(cpStyleA->GetCode());
    expectedCodes.insert(cpStyleB->GetCode());
    ExpectCodes(expectedCodes, createdCodes);

    // Change two codes, reusing one. Because the revision is composed of *net* changes, we should not see the reused code.
    auto codeB = cpStyleB->GetCode();
    cpStyleA = RenameTextStyle(*cpStyleA, "C");
    cpStyleB = RenameTextStyle(*cpStyleB, "A");

    ExtractCodesFromRevision(createdCodes, discardedCodes);
    expectedCodes.clear();
    expectedCodes.insert(codeB);
    ExpectCodes(expectedCodes, discardedCodes);

    expectedCodes.clear();
    expectedCodes.insert(cpStyleA->GetCode());
    ExpectCodes(expectedCodes, createdCodes);

    // Create two elements with a code, and one with a default (empty) code. We only care about non-empty codes.
    auto defaultCode = DgnCode::CreateEmpty();
    auto codeSpec = CodeSpec::Create(db, "MyCodeSpec");
    EXPECT_EQ(DgnDbStatus::Success, codeSpec->Insert());

    auto cpElX1 = InsertPhysicalElementByCode(codeSpec->CreateCode("X")),
        cpElY2 = InsertPhysicalElementByCode(codeSpec->CreateCode("Y")),
        cpUncoded = InsertPhysicalElementByCode(defaultCode);

    ExtractCodesFromRevision(createdCodes, discardedCodes);

    expectedCodes.clear();
    ExpectCodes(expectedCodes, discardedCodes);

    expectedCodes.insert(cpElX1->GetCode());
    expectedCodes.insert(cpElY2->GetCode());
    ExpectCodes(expectedCodes, createdCodes);

    // Set one code to empty, and one empty code to a non-empty code, and delete one coded element, and create a new element with the same code as the deleted element
    cpUncoded = RenameElement(*cpUncoded, codeSpec->CreateCode("Z"));
    auto codeX1 = cpElX1->GetCode();
    cpElX1 = RenameElement(*cpElX1, defaultCode);
    auto codeY2 = cpElY2->GetCode();
    EXPECT_EQ(DgnDbStatus::Success, cpElY2->Delete());
    auto cpNewElY2 = InsertPhysicalElementByCode(codeY2);

    // The code that was set to empty should be seen as discarded; the code that replaced empty should be seen as new; the reused code should not appear.
    ExtractCodesFromRevision(createdCodes, discardedCodes);

    expectedCodes.clear();
    expectedCodes.insert(codeX1);
    ExpectCodes(expectedCodes, discardedCodes);

    expectedCodes.clear();
    expectedCodes.insert(cpUncoded->GetCode());
    ExpectCodes(expectedCodes, createdCodes);
    }

//=======================================================================================
// Silly handler for testing dependencies.
// Assuming A and B are TestElement-s, if A drives B then:
//  - TestElementDrivesElement.Property1 is an integer X from 0-3 identifying TestIntegerProperty[X+1]
//    in element B
//  - The value of TestIntegerPropertyX always has the same value as A.TestIntegerProperty1
// @bsistruct                                                   Paul.Connelly   05/16
//=======================================================================================
struct TestElementDependency : TestElementDrivesElementHandler::Callback
{
    int32_t m_mostRecentValue = -1;
    uint32_t m_invocationCount = 0;

    void Reset() { m_mostRecentValue = -1; m_invocationCount = 0; }
    int32_t GetMostRecentValue() const { return m_mostRecentValue; }
    uint32_t GetInvocationCount() const { return m_invocationCount; }

    void _OnRootChanged(DgnDbR, ECInstanceId, DgnElementId, DgnElementId) override;
    void _ProcessDeletedDependency(DgnDbR, dgn_TxnTable::ElementDep::DepRelData const&) override { }

    TestElementDependency() { TestElementDrivesElementHandler::SetCallback(this); }
    ~TestElementDependency() { TestElementDrivesElementHandler::SetCallback(nullptr); }

    static void Insert(DgnDbR db, DgnElementId rootId, DgnElementId depId, uint8_t index);
    static uint8_t GetIndex(DgnDbR db, ECInstanceId relId);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElementDependency::Insert(DgnDbR db, DgnElementId rootId, DgnElementId depId, uint8_t index)
    {
    ECInstanceKey key = TestElementDrivesElementHandler::Insert(db, rootId, depId);
    Utf8CP str = "0";
    switch (index)
        {
        case 1: str = "1"; break;
        case 2: str = "2"; break;
        case 3: str = "3"; break;
        }

    TestElementDrivesElementHandler::SetProperty1(db, str, key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint8_t TestElementDependency::GetIndex(DgnDbR db, ECInstanceId relId)
    {
    Utf8String str = TestElementDrivesElementHandler::GetProperty1(db, relId);
    uint8_t idx = 0;
    if (0 < str.length())
        {
        switch (str[0])
            {
            case '1':   idx = 1; break;
            case '2':   idx = 2; break;
            case '3':   idx = 3; break;
            }
        }

    return idx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TestElementDependency::_OnRootChanged(DgnDbR db, ECInstanceId relId, DgnElementId rootId, DgnElementId depId)
    {
    ++m_invocationCount;
    auto root = db.Elements().Get<TestElement>(rootId);
    auto dep = db.Elements().GetForEdit<TestElement>(depId);
    ASSERT_TRUE(root.IsValid() && dep.IsValid());

    uint8_t index = GetIndex(db, relId);
    m_mostRecentValue = root->GetIntegerProperty(0);
    dep->SetIntegerProperty(index, m_mostRecentValue);
    auto cpDep = db.Elements().Update(*dep);
    ASSERT_TRUE(cpDep.IsValid());
    EXPECT_EQ(cpDep->GetIntegerProperty(index), root->GetIntegerProperty(0));
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/16
//=======================================================================================
struct DependencyRevisionTest : RevisionTestFixture
{
    DEFINE_T_SUPER(RevisionTestFixture);

    TestElementDependency   m_dep;

    TestElementCPtr InsertElement(int32_t intProp1);
    void UpdateRootProperty(int32_t intProp1, DgnElementId eId);
    void VerifyRootProperty(DgnElementId, int32_t);
    void UpdateDependentProperty(DgnElementId eId, uint8_t index, int32_t value);
    void VerifyDependentProperty(DgnElementId eId, uint8_t index, int32_t value);
    void VerifyDependentProperties(DgnElementId, std::array<int32_t, 4> const&);

    DependencyRevisionTest() { }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementCPtr DependencyRevisionTest::InsertElement(int32_t intProp1)
    {
    DgnDbR db = m_defaultModel->GetDgnDb();
    auto el = TestElement::Create(db, m_defaultModelId, m_defaultCategoryId);
    el->SetIntegerProperty(0, intProp1);
    auto cpEl = db.Elements().Insert(*el);
    EXPECT_TRUE(cpEl.IsValid());
    return cpEl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyRevisionTest::UpdateRootProperty(int32_t intProp1, DgnElementId eId)
    {
    auto el = m_db->Elements().GetForEdit<TestElement>(eId);
    ASSERT_TRUE(el.IsValid());
    el->SetIntegerProperty(0, intProp1);
    ASSERT_TRUE(el->Update().IsValid());
    VerifyRootProperty(eId, intProp1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyRevisionTest::VerifyRootProperty(DgnElementId eId, int32_t prop)
    {
    auto el = m_db->Elements().Get<TestElement>(eId);
    ASSERT_TRUE(el.IsValid());
    EXPECT_EQ(el->GetIntegerProperty(0), prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyRevisionTest::UpdateDependentProperty(DgnElementId eId, uint8_t index, int32_t value)
    {
    auto el = m_db->Elements().GetForEdit<TestElement>(eId);
    ASSERT_TRUE(el.IsValid());
    el->SetIntegerProperty(index, value);
    ASSERT_TRUE(el->Update().IsValid());
    VerifyDependentProperty(eId, index, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyRevisionTest::VerifyDependentProperty(DgnElementId eId, uint8_t index, int32_t value)
    {
    auto el = m_db->Elements().Get<TestElement>(eId);
    ASSERT_TRUE(el.IsValid());
    EXPECT_EQ(el->GetIntegerProperty(index), value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyRevisionTest::VerifyDependentProperties(DgnElementId eId, std::array<int32_t, 4> const& props)
    {
    auto el = m_db->Elements().Get<TestElement>(eId);
    ASSERT_TRUE(el.IsValid());
    EXPECT_EQ(el->GetIntegerProperty(0), props[0]);
    EXPECT_EQ(el->GetIntegerProperty(1), props[1]);
    EXPECT_EQ(el->GetIntegerProperty(2), props[2]);
    EXPECT_EQ(el->GetIntegerProperty(3), props[3]);
    }

/*---------------------------------------------------------------------------------**//**
* Ensure the dependency callback works as expected...
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DependencyRevisionTest, TestDependency)
    {
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"TestDependency.bim");
    auto& db = *m_db;
    db.SaveChanges("Create initial model");

    // C.TestIntegerProperty2 is driven by A.TestIntegerProperty1
    // C.TestIntegerProperty3 is driven by B.TestIntegerProperty1
    auto a = InsertElement(123),
         b = InsertElement(456),
         c = InsertElement(789);

    auto aId = a->GetElementId(),
         bId = b->GetElementId(),
         cId = c->GetElementId();

    TestElementDependency::Insert(db, aId, cId, 2);
    TestElementDependency::Insert(db, bId, cId, 3);

    db.SaveChanges("Initial dependencies");

    c = db.Elements().Get<TestElement>(cId);
    EXPECT_EQ(123, c->GetIntegerProperty(2));
    EXPECT_EQ(456, c->GetIntegerProperty(3));

    auto pA = db.Elements().GetForEdit<TestElement>(aId);
    auto pB = db.Elements().GetForEdit<TestElement>(bId);
    pA->SetIntegerProperty(0, 321);
    EXPECT_TRUE(pA->Update().IsValid());
    pB->SetIntegerProperty(0, 654);
    EXPECT_TRUE(pB->Update().IsValid());

    EXPECT_EQ(321, pA->GetIntegerProperty(0));
    EXPECT_EQ(654, pB->GetIntegerProperty(0));

    m_dep.Reset();
    EXPECT_EQ(0, m_dep.GetInvocationCount());

    db.SaveChanges("Modify root properties");

    // sporadically, and apparently only in optimized builds, dependent element C's properties do not get updated, as if the dependency callback was never invoked.
    // Check that the dependency callback was invoked
    EXPECT_EQ(2, m_dep.GetInvocationCount());

    // Check that the root properties at least were saved to eliminate that possibility
    a = db.Elements().Get<TestElement>(aId);
    b = db.Elements().Get<TestElement>(bId);
    EXPECT_EQ(321, a->GetIntegerProperty(0));
    EXPECT_EQ(654, b->GetIntegerProperty(0));

    c = db.Elements().Get<TestElement>(cId);
    EXPECT_EQ(321, c->GetIntegerProperty(2));
    EXPECT_EQ(654, c->GetIntegerProperty(3));
    }

/*---------------------------------------------------------------------------------**//**
* Create a revision which includes indirect changes, then apply that revision.
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DependencyRevisionTest, SingleRevision)
    {
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"SingleRevision.bim");

    DgnElementId aId, bId, cId;

    // set up initial dependencies
    aId = InsertElement(123)->GetElementId();
    bId = InsertElement(456)->GetElementId();
    cId = InsertElement(789)->GetElementId();

    TestElementDependency::Insert(*m_db, aId, cId, 2);
    TestElementDependency::Insert(*m_db, bId, cId, 3);

    // Save initial state
    m_db->SaveChanges();
    VerifyDependentProperties(cId, { 789, 0, 123, 456 });
    ASSERT_TRUE(CreateRevision().IsValid());
    BackupTestFile();
    VerifyDependentProperties(cId, { 789, 0, 123, 456 });

    // Create a revision involving indirect changes
    UpdateRootProperty(321, aId);
    UpdateRootProperty(654, bId);
    m_db->SaveChanges("Modify root properties");

    VerifyDependentProperties(cId, { 789, 0, 321, 654 });
    DgnRevisionPtr rev = CreateRevision();
    ASSERT_TRUE(rev.IsValid());
    DumpRevision(*rev);

    // Restore the initial state of the db and apply the revision
    RestoreTestFile();
    VerifyDependentProperties(cId, { 789, 0, 123, 456 });
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*rev));
    m_db->SaveChanges("Applied revision");
    VerifyDependentProperties(cId, { 789, 0, 321, 654 });
    VerifyRootProperty(aId, 321);
    VerifyRootProperty(bId, 654);
    }

/*---------------------------------------------------------------------------------**//**
* Two revisions which indirectly modify the same element in different ways should
* produce consistent results regardless of the order in which they are merged.
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DependencyRevisionTest, Merge)
    {
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"Merge.bim");

    DgnElementId aId, bId, cId;

    // set up initial dependencies
    aId = InsertElement(123)->GetElementId();
    bId = InsertElement(456)->GetElementId();
    cId = InsertElement(789)->GetElementId();

    TestElementDependency::Insert(*m_db, aId, cId, 2);
    TestElementDependency::Insert(*m_db, bId, cId, 3);

    // Save initial state
    m_db->SaveChanges();
    VerifyDependentProperties(cId, { 789, 0, 123, 456 });
    ASSERT_TRUE(CreateRevision().IsValid());
    BackupTestFile();
    VerifyDependentProperties(cId, { 789, 0, 123, 456 });

    // Create a revision which modifies only element A
    UpdateRootProperty(321, aId);
    m_db->SaveChanges("Modify element A");
    VerifyDependentProperties(cId, { 789, 0, 321, 456 });
    DgnRevisionPtr revA = CreateRevision();
    ASSERT_TRUE(revA.IsValid());
    VerifyRootProperty(aId, 321);
    DumpRevision(*revA, "Revision A");

    // Restore initial state, merge in A, modify B, and save as revision AB
    RestoreTestFile();
    VerifyDependentProperties(cId, { 789, 0, 123, 456 });
    VerifyRootProperty(aId, 123);
    VerifyRootProperty(bId, 456);
    LOG.infov("Merging Revision A");
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*revA));
    m_db->SaveChanges("Merge A");
    VerifyRootProperty(aId, 321);
    VerifyDependentProperties(cId, { 789, 0, 321, 456 });
    m_db->Elements().ClearCache();
    VerifyRootProperty(aId, 321);

    UpdateRootProperty(654, bId);
    m_db->SaveChanges("Modify element B");
    VerifyDependentProperties(cId, { 789, 0, 321, 654 });
    VerifyRootProperty(bId, 654);

    DgnRevisionPtr revAB = CreateRevision();
    ASSERT_TRUE(revAB.IsValid());
    DumpRevision(*revAB, "Revision AB");

    // Restore initial state, modify B, merge in A, and save as revision BA
    RestoreTestFile();
    UpdateRootProperty(654, bId);
    m_db->SaveChanges("Modify element B");
    VerifyDependentProperties(cId, { 789, 0, 123, 654 });
    VerifyRootProperty(bId, 654);
    VerifyRootProperty(aId, 123);

    LOG.infov("Merging Revision A");
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*revA));
    m_db->SaveChanges("Merge A");
    VerifyDependentProperties(cId, { 789, 0, 321, 654 });
    VerifyRootProperty(aId, 321);
    VerifyRootProperty(bId, 654);

    DgnRevisionPtr revBA = CreateRevision();
    ASSERT_TRUE(revBA.IsValid());
    DumpRevision(*revBA, "Revision BA");

    // Note: The tests below ensure that the order of merges A->AB or A->BA doesn't affect
    // the final state of the properties of A, B and C. 
    
    // Restore initial state, merge in AB
    RestoreTestFile();
    LOG.infov("Merging Revision A");
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*revA));
    LOG.infov("Merging Revision AB");
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*revAB));
    m_db->SaveChanges("Merge AB");
    VerifyDependentProperties(cId, { 789, 0, 321, 654 });
    VerifyRootProperty(aId, 321);
    VerifyRootProperty(bId, 654);

    // Restore initial state, merge in BA
    RestoreTestFile();
    LOG.infov("Merging Revision A");
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*revA));
    LOG.infov("Merging Revision BA");
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*revBA));
    m_db->SaveChanges("Merge BA");
    VerifyDependentProperties(cId, { 789, 0, 321, 654 });
    VerifyRootProperty(aId, 321);
    VerifyRootProperty(bId, 654);
    }

/*---------------------------------------------------------------------------------**//**
* Elements A and B both drive the same property of Element C. The results will be
* ambiguous (whatever dependency is processed most recently will win).
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DependencyRevisionTest, Conflict)
    {
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"Conflict.bim");
    DgnElementId aId = InsertElement(123)->GetElementId(),
                 bId = InsertElement(456)->GetElementId(),
                 cId = InsertElement(789)->GetElementId();

    TestElementDependency::Insert(*m_db, aId, cId, 1);
    TestElementDependency::Insert(*m_db, bId, cId, 1);

    m_dep.Reset();
    m_db->SaveChanges();
    VerifyDependentProperties(cId, { 789, m_dep.GetMostRecentValue(), 0, 0 });
    ASSERT_TRUE(CreateRevision().IsValid());
    BackupTestFile();

    m_dep.Reset();
    UpdateRootProperty(321, aId);
    m_db->SaveChanges("Modify A");
    VerifyDependentProperties(cId, { 789, m_dep.GetMostRecentValue(), 0, 0 });
    DgnRevisionPtr revA = CreateRevision();
    ASSERT_TRUE(revA.IsValid());

    m_dep.Reset();
    RestoreTestFile();
    UpdateRootProperty(654, bId);
    m_db->SaveChanges("Modify B");
    VerifyDependentProperties(cId, { 789, m_dep.GetMostRecentValue(), 0, 0 });

    m_dep.Reset();
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*revA));
    m_db->SaveChanges("Merge A");
    VerifyRootProperty(aId, 321);
    UpdateRootProperty(987, bId);
    m_db->SaveChanges("Modify B again");
    VerifyDependentProperties(cId, { 789, m_dep.GetMostRecentValue(), 0, 0 });
    }

/*---------------------------------------------------------------------------------**//**
* Directly modify a dependent element, then run dependency callbacks which indirectly
* modify the dependent element in a different way.
* Expect that no conflicts occur in merging revisions containing these changes.
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DependencyRevisionTest, DirectAndIndirectChangesToSameElement)
    {
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"DirectAndIndirectChangesToSameElement.bim");

    // Value of dependent's TestIntegerProperty2 == root's TestIntegerProperty1
    DgnElementId rootId = InsertElement(123)->GetElementId();
    DgnElementId depId = InsertElement(456)->GetElementId();
    TestElementDependency::Insert(*m_db, rootId, depId, 2);

    // Save initial state
    m_db->SaveChanges();
    ASSERT_TRUE(CreateRevision().IsValid());
    VerifyDependentProperties(depId, { 456, 0, 123, 0 });
    BackupTestFile();

    // Modify dependent property directly
    UpdateDependentProperty(depId, 2, 789);
    VerifyDependentProperties(depId, { 456, 0, 789, 0 });

    // Saving changes will re-run dependency logic
    m_db->SaveChanges();
    VerifyDependentProperties(depId, { 456, 0, 123, 0 });

    // Create a revision
    DgnRevisionPtr rev = CreateRevision();
    ASSERT_TRUE(rev.IsValid());
    DumpRevision(*rev);

    // Apply revision to original state - expect same state + no conflicts
    RestoreTestFile();
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*rev));
    m_db->SaveChanges();
    VerifyDependentProperties(depId, { 456, 0, 123, 0 });
    }

/*---------------------------------------------------------------------------------**//**
* When a changeset is applied for undo/redo, we notify TxnTables so they can update
* their in-memory state. e.g., undoing the insertion of an element causes that element
* to be dropped from the element cache.
* This same logic was not being applied when merging revisions. Verify that it now is
* applied correctly.
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DependencyRevisionTest, UpdateCache)
    {
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"UpdateCache.bim");
    
    // Initial state: create two elements
    DgnElementId aId = InsertElement(123)->GetElementId();
    DgnElementId bId = InsertElement(456)->GetElementId();
    m_db->SaveChanges();
    ASSERT_TRUE(CreateRevision().IsValid());
    BackupTestFile();

    // Revision: delete element A; modify element B
    EXPECT_EQ(DgnDbStatus::Success, m_db->Elements().Get<TestElement>(aId)->Delete());
    UpdateRootProperty(654, bId);
    m_db->SaveChanges("Revision");
    VerifyRootProperty(bId, 654);
    EXPECT_TRUE(m_db->Elements().Get<TestElement>(aId).IsNull());

    DgnRevisionPtr rev = CreateRevision();
    ASSERT_TRUE(rev.IsValid());

    // Restore initial state and verify it
    RestoreTestFile();
    auto elA = m_db->Elements().Get<TestElement>(aId);
    auto elB = m_db->Elements().Get<TestElement>(bId);
    ASSERT_TRUE(elA.IsValid());
    ASSERT_TRUE(elB.IsValid());
    EXPECT_TRUE(elA->IsPersistent());
    EXPECT_TRUE(elB->IsPersistent());
    EXPECT_EQ(456, elB->GetIntegerProperty(0));
    VerifyRootProperty(bId, 456);

    // Apply revision and verify cache
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*rev));
    EXPECT_TRUE(m_db->Elements().Get<TestElement>(aId).IsNull());
    EXPECT_FALSE(m_db->Elements().Get<TestElement>(aId).IsValid());
    EXPECT_FALSE(elA->IsPersistent());
    EXPECT_TRUE(elB->IsPersistent());   // NB: The original element remains in the cache, now modified to match updated state...
    EXPECT_EQ(654, elB->GetIntegerProperty(0));
    VerifyRootProperty(bId, 654);

    elA = nullptr;
    elB = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/2016
//---------------------------------------------------------------------------------------
TEST_F(DependencyRevisionTest, MergeDependencyPermutations)
    {
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"MergeDependencyPermutations.bim");
    
    RevisionStatus status;

    // Value of dependent's TestIntegerProperty2 == root's TestIntegerProperty1
    DgnElementId rootId = InsertElement(123)->GetElementId();
    DgnElementId depId = InsertElement(456)->GetElementId();
    TestElementDependency::Insert(*m_db, rootId, depId, 2);

    // Save initial state
    m_db->SaveChanges();
    ASSERT_TRUE(CreateRevision().IsValid());
    VerifyDependentProperties(depId, {456, 0, 123, 0});
    BackupTestFile();

    /*
     * Create a revision with some direct changes to the dependent property
     */
    UpdateDependentProperty(depId, 2, 789);
    VerifyDependentProperties(depId, {456, 0, 789, 0});
    m_db->SaveChanges();
    VerifyDependentProperties(depId, {456, 0, 123, 0});

    DgnRevisionPtr directChangesRev = CreateRevision(); // Will only contain LastMod change
    ASSERT_TRUE(directChangesRev.IsValid());
    DumpRevision(*directChangesRev, "DirectChangesRev:");

    /*
     * Create a revision with some indirect changes to the dependent property
     */
    RestoreTestFile();
    UpdateRootProperty(654, rootId);
    m_db->SaveChanges("Revision");
    VerifyRootProperty(rootId, 654);
    VerifyDependentProperties(depId, {456, 0, 654, 0});

    DgnRevisionPtr indirectChangesRev = CreateRevision();
    ASSERT_TRUE(indirectChangesRev.IsValid());
    DumpRevision(*indirectChangesRev, "IndirectChangesRev:");

    /*
     * Make direct changes, and merge the revision with indirect changes
     */
    RestoreTestFile();
    UpdateDependentProperty(depId, 2, 789);
    m_db->SaveChanges();
    VerifyDependentProperties(depId, {456, 0, 123, 0});
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*indirectChangesRev));
    VerifyDependentProperties(depId, {456, 0, 654, 0});

    // Create new revision
    DgnRevisionPtr directAndIndirectChangesRev = CreateRevision(); // Will only contain LastMod change
    ASSERT_TRUE(directAndIndirectChangesRev.IsValid());
    DumpRevision(*directAndIndirectChangesRev, "DirectAndIndirectChangesRev:");

    // Test reverse
    status = m_db->Revisions().ReverseRevision(*directAndIndirectChangesRev);
    ASSERT_TRUE(RevisionStatus::Success == status);
    VerifyRootProperty(rootId, 654);
    VerifyDependentProperties(depId, {456, 0, 654, 0});

    status = m_db->Revisions().ReverseRevision(*indirectChangesRev);
    ASSERT_TRUE(RevisionStatus::Success == status);
    VerifyRootProperty(rootId, 123);
    VerifyDependentProperties(depId, {456, 0, 123, 0});

    // Test reinstate
    status = m_db->Revisions().ReinstateRevision(*indirectChangesRev);
    ASSERT_TRUE(RevisionStatus::Success == status);
    VerifyRootProperty(rootId, 654);
    VerifyDependentProperties(depId, {456, 0, 654, 0});

    status = m_db->Revisions().ReinstateRevision(*directAndIndirectChangesRev);
    ASSERT_TRUE(RevisionStatus::Success == status);
    VerifyRootProperty(rootId, 654);
    VerifyDependentProperties(depId, {456, 0, 654, 0});

    /*
     * Make indirect changes, and merge the revision with direct changes
     */
    RestoreTestFile();
    UpdateRootProperty(654, rootId);
    m_db->SaveChanges("Revision");
    VerifyRootProperty(rootId, 654);
    VerifyDependentProperties(depId, {456, 0, 654, 0});
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*directChangesRev));
    VerifyDependentProperties(depId, {456, 0, 654, 0});

    // Create a new revision
    DgnRevisionPtr indirectAndDirectChangesRev = CreateRevision();
    ASSERT_TRUE(indirectAndDirectChangesRev.IsValid());
    DumpRevision(*indirectAndDirectChangesRev, "IndirectAndDirectChangesRev:");

    // Test reverse
    status = m_db->Revisions().ReverseRevision(*indirectAndDirectChangesRev);
    ASSERT_TRUE(RevisionStatus::Success == status);
    VerifyRootProperty(rootId, 123);
    VerifyDependentProperties(depId, {456, 0, 123, 0});

    status = m_db->Revisions().ReverseRevision(*directChangesRev);
    ASSERT_TRUE(RevisionStatus::Success == status);
    VerifyRootProperty(rootId, 123);
    VerifyDependentProperties(depId, {456, 0, 123, 0});

    // Test reinstate
    status = m_db->Revisions().ReinstateRevision(*directChangesRev);
    ASSERT_TRUE(RevisionStatus::Success == status);
    VerifyRootProperty(rootId, 123);
    VerifyDependentProperties(depId, {456, 0, 123, 0});

    status = m_db->Revisions().ReinstateRevision(*indirectAndDirectChangesRev);
    ASSERT_TRUE(RevisionStatus::Success == status);
    VerifyRootProperty(rootId, 654);
    VerifyDependentProperties(depId, {456, 0, 654, 0});
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
bool ValidateValue(DgnDbCR db, Utf8CP sql, int expectedValue)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(db, sql))
        {
        BeAssert(false);
        return false;
        }

    if (BE_SQLITE_ROW != stmt.Step())
        {
        BeAssert(false);
        return false;
        }

    return (expectedValue == stmt.GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, ReverseAndReinstate)
    {
    /* Setup baseline */
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"ReverseAndReinstate.bim");
    m_db->CreateTable("TestTable", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable(Id, Column1) VALUES(1,0)"), BE_SQLITE_OK);
    m_db->SaveChanges("Created Initial Model");
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    RevisionManagerR revMgr = m_db->Revisions();

    /* Create Revision 1 */
    ASSERT_EQ(m_db->ExecuteSql("UPDATE TestTable SET Column1=1 WHERE Id=1"), BE_SQLITE_OK);
    m_db->SaveChanges("Revision 1");
    DgnRevisionPtr revision1 = CreateRevision();
    ASSERT_TRUE(revision1.IsValid());

    /* Create Revision 2 */
    ASSERT_EQ(m_db->ExecuteSql("UPDATE TestTable SET Column1=2 WHERE Id=1"), BE_SQLITE_OK);
    m_db->SaveChanges("Revision 2");
    DgnRevisionPtr revision2 = CreateRevision();
    ASSERT_TRUE(revision2.IsValid());

    // Validate
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable WHERE Id=1", 2));
    ASSERT_FALSE(revMgr.HasReversedRevisions());
    ASSERT_STREQ(revision2->GetId().c_str(), revMgr.GetParentRevisionId().c_str());
    ASSERT_TRUE(revMgr.GetReversedRevisionId().empty());

    /* Reverse Revision 2 */
    RevisionStatus status = revMgr.ReverseRevision(*revision2);
    ASSERT_TRUE(RevisionStatus::Success == status);

    // Validate
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable WHERE Id=1", 1));
    ASSERT_TRUE(revMgr.HasReversedRevisions());
    ASSERT_STREQ(revision2->GetId().c_str(), revMgr.GetParentRevisionId().c_str());
    ASSERT_STREQ(revision1->GetId().c_str(), revMgr.GetReversedRevisionId().c_str());

    /* Reverse Revision 1 */
    status = revMgr.ReverseRevision(*revision1);
    ASSERT_TRUE(RevisionStatus::Success == status);

    // Validate
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable WHERE Id=1", 0));
    ASSERT_TRUE(revMgr.HasReversedRevisions());
    ASSERT_STREQ(revision2->GetId().c_str(), revMgr.GetParentRevisionId().c_str());
    ASSERT_STREQ(initialRevision->GetId().c_str(), revMgr.GetReversedRevisionId().c_str());

    // Commit with reversed revisions should fail
    ASSERT_EQ(m_db->ExecuteSql("UPDATE TestTable SET Column1=108 WHERE Id=1"), BE_SQLITE_OK);
    BeTest::SetFailOnAssert(false);
    DbResult result = m_db->SaveChanges("Revision Invalid");
    BeTest::SetFailOnAssert(true);
    ASSERT_NE(BE_SQLITE_OK, result);
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable WHERE Id=1", 108));
    result = m_db->AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, result);
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable WHERE Id=1", 0));

    /* Reinstate Revision 1 */
    status = revMgr.ReinstateRevision(*revision1);
    ASSERT_TRUE(RevisionStatus::Success == status);

    // Validate
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable WHERE Id=1", 1));
    ASSERT_TRUE(revMgr.HasReversedRevisions());
    ASSERT_STREQ(revision2->GetId().c_str(), revMgr.GetParentRevisionId().c_str());
    ASSERT_STREQ(revision1->GetId().c_str(), revMgr.GetReversedRevisionId().c_str());

    /* Reinstate Revision 2 */
    status = revMgr.ReinstateRevision(*revision2);
    ASSERT_TRUE(RevisionStatus::Success == status);

    // Validate
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable WHERE Id=1", 2));
    ASSERT_FALSE(revMgr.HasReversedRevisions());
    ASSERT_STREQ(revision2->GetId().c_str(), revMgr.GetParentRevisionId().c_str());
    ASSERT_TRUE(revMgr.GetReversedRevisionId().empty());

    // Commit with reinstated revisions should succeed again
    ASSERT_EQ(m_db->ExecuteSql("UPDATE TestTable SET Column1=108 WHERE Id=1"), BE_SQLITE_OK);
    result = m_db->SaveChanges("Revision 3");
    ASSERT_EQ(BE_SQLITE_OK, result);
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable WHERE Id=1", 108));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, DbSchemaChanges)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"SchemaChanges.bim");
    m_db->SaveChanges("Created Initial Model");
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    // Create Revision 1 (Schema changes - creating two tables)
    m_db->CreateTable("TestTable1", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->CreateTable("TestTable2", "Id INTEGER PRIMARY KEY, Column1 INTEGER");

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDbSchemaChanges());
    ASSERT_TRUE(m_db->Txns().HasChanges());

    m_db->SaveChanges("Revision 1");
    DgnRevisionPtr revision1 = CreateRevision();
    ASSERT_TRUE(revision1.IsValid());
    DumpRevision(*revision1, "Revision 1 with only schema changes:");

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDbSchemaChanges());
    ASSERT_FALSE(m_db->Txns().HasChanges());

    // Create Revision 2 (Data changes - inserts to both tables)
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable1(Id, Column1) VALUES(1,1)"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable2(Id, Column1) VALUES(1,1)"), BE_SQLITE_OK);

    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDbSchemaChanges());
    ASSERT_TRUE(m_db->Txns().HasChanges());

    m_db->SaveChanges("Revision 2");
    DgnRevisionPtr revision2 = CreateRevision();
    ASSERT_TRUE(revision2.IsValid());
    DumpRevision(*revision2, "Revision 2 with only data changes:");

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDbSchemaChanges());
    ASSERT_FALSE(m_db->Txns().HasChanges());

    // Create Revision 3 (Schema changes to first table, and data changes to the second)
    ASSERT_EQ(m_db->AddColumnToTable("TestTable1", "Column2", "INTEGER"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->CreateIndex("idx_TestTable1_Column1", "TestTable1", false, "Column1"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable2(Id, Column1) VALUES(2,2)"), BE_SQLITE_OK);

    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDbSchemaChanges());
    ASSERT_TRUE(m_db->Txns().HasChanges());

    m_db->SaveChanges("Revision 3");
    DgnRevisionPtr revision3 = CreateRevision();
    ASSERT_TRUE(revision3.IsValid());
    DumpRevision(*revision3, "Revision 3 with schema and data changes:");

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDbSchemaChanges());
    ASSERT_FALSE(m_db->Txns().HasChanges());

    // Create Revision 4 (Data changes to the first table, and schema changes to the other)
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable1(Id, Column1, Column2) VALUES(2,2,2)"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->ExecuteSql("UPDATE TestTable1 SET Column2=1 WHERE Id=1"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->AddColumnToTable("TestTable2", "Column2", "INTEGER"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->CreateIndex("idx_TestTable2_Column1", "TestTable2", false, "Column1"), BE_SQLITE_OK);

    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDbSchemaChanges());
    ASSERT_TRUE(m_db->Txns().HasChanges());

    m_db->SaveChanges("Revision 4");
    DgnRevisionPtr revision4 = CreateRevision();
    ASSERT_TRUE(revision4.IsValid());
    DumpRevision(*revision4, "Revision 4 with schema and data changes:");

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDbSchemaChanges());
    ASSERT_FALSE(m_db->Txns().HasChanges());

    // Revert Db to initial state
    RestoreTestFile();

    // Merge revision 1 (Schema changes - creating two tables)
    LOG.infov("Merging Revision 1");
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(RevisionStatus::MergeSchemaChangesOnOpen, m_db->Revisions().MergeRevision(*revision1));
    BeTest::SetFailOnAssert(true);
    MergeSchemaRevision(*revision1);

    ASSERT_TRUE(m_db->TableExists("TestTable1"));
    ASSERT_TRUE(m_db->TableExists("TestTable2"));

    ASSERT_TRUE(m_db->ColumnExists("TestTable1", "Id"));
    ASSERT_TRUE(m_db->ColumnExists("TestTable1", "Column1"));
    ASSERT_FALSE(m_db->ColumnExists("TestTable1", "Column2"));

    ASSERT_TRUE(m_db->ColumnExists("TestTable2", "Id"));
    ASSERT_TRUE(m_db->ColumnExists("TestTable2", "Column1"));
    ASSERT_FALSE(m_db->ColumnExists("TestTable2", "Column2"));

    // Merge revision 2 (Data changes - inserts to both tables)
    LOG.infov("Merging Revision 2");
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*revision2));

    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable1 WHERE Id=1", 1));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable2 WHERE Id=1", 1));

    // Merge revision 3 (Schema changes to first table, and data changes to the second)
    LOG.infov("Merging Revision 3");
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(RevisionStatus::MergeSchemaChangesOnOpen, m_db->Revisions().MergeRevision(*revision3));
    BeTest::SetFailOnAssert(true);
    MergeSchemaRevision(*revision3);

    ASSERT_TRUE(m_db->ColumnExists("TestTable1", "Column2"));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable1 WHERE Id=1", 0)); // i.e., null value

    ASSERT_FALSE(m_db->ColumnExists("TestTable2", "Column2"));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable2 WHERE Id=2", 2));

    // Merge revision 4 (Data changes to the first table, and schema changes to the other)
    LOG.infov("Merging Revision 4");
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(RevisionStatus::MergeSchemaChangesOnOpen, m_db->Revisions().MergeRevision(*revision4));
    BeTest::SetFailOnAssert(true);
    MergeSchemaRevision(*revision4);

    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable1 WHERE Id=1", 1));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable1 WHERE Id=2", 2));

    ASSERT_TRUE(m_db->ColumnExists("TestTable2", "Column2"));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable2 WHERE Id=1", 0)); // i.e., null value
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable2 WHERE Id=2", 0)); // i.e., null value

    // Reverse revision 4
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(RevisionStatus::ReverseOrReinstateSchemaChangesOnOpen, m_db->Revisions().ReverseRevision(*revision4));
    BeTest::SetFailOnAssert(true);
    ReverseSchemaRevision(*revision4);
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable1 WHERE Id=1", 0)); // i.e., null value

    // Reinstate revision 4
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(RevisionStatus::ReverseOrReinstateSchemaChangesOnOpen, m_db->Revisions().ReinstateRevision(*revision4));
    BeTest::SetFailOnAssert(true);
    ReinstateSchemaRevision(*revision4);
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable1 WHERE Id=1", 1)); // i.e., null value
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, InvalidSchemaChanges)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"SchemaChanges.bim");
    m_db->SaveChanges("Created Initial Model");
   
    ASSERT_TRUE(BE_SQLITE_OK == m_db->CreateTable("TestTable", "Id INTEGER PRIMARY KEY, Column1 INTEGER"));
    m_db->SaveChanges("Created test table");
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    // Make invalid schema changes (using the schema change API)
    BeTest::SetFailOnAssert(false);
    ASSERT_TRUE(BE_SQLITE_ERROR == m_db->RenameTable("TestTable", "TestTableWontHappen")); // Fails
    ASSERT_TRUE(BE_SQLITE_ERROR == m_db->DropTable("TestTable")); // Fails
    BeTest::SetFailOnAssert(true);

    // Make invalid schema changes (outside the schema change API)
    ASSERT_TRUE(BE_SQLITE_OK == m_db->ExecuteSql("ALTER TABLE TestTable RENAME TO TestTableWillHappen")); // Unfortunately succeeds. Need a way to monitor DDL changes. 

    // Make schema changes with tracking disabled
    m_db->Txns().EnableTracking(false);
    ASSERT_TRUE(BE_SQLITE_OK == m_db->DropTable("TestTableWillHappen"));
    m_db->Txns().EnableTracking(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, ManySchemaChanges)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"SchemaChanges.bim");
    m_db->SaveChanges("Created Initial Model");
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    // Create a revision with a number of new tables and columns
    int numTables = 0;
    while (++numTables <= 10)
        {
        Utf8PrintfString tableName("TestTable%d", numTables);
        ASSERT_TRUE(BE_SQLITE_OK == m_db->CreateTable(tableName.c_str(), "Id INTEGER PRIMARY KEY, Column0 INTEGER"));

        int numColumns = 0;
        while (++numColumns <= 10)
            {
            Utf8PrintfString columnName("Column%d", numColumns);
            ASSERT_EQ(m_db->AddColumnToTable(tableName.c_str(), columnName.c_str(), "INTEGER"), BE_SQLITE_OK);
            }
        }

    m_db->SaveChanges("");
    DgnRevisionPtr revision = CreateRevision();
    ASSERT_TRUE(revision.IsValid());
    DumpRevision(*revision, "Revision containing many schema changes:");

    // Revert Db to initial state
    RestoreTestFile();

    // Merge revision 1
    LOG.infov("Merging revision containing many schema changes");
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(RevisionStatus::MergeSchemaChangesOnOpen, m_db->Revisions().MergeRevision(*revision));
    BeTest::SetFailOnAssert(true);
    MergeSchemaRevision(*revision);

    ASSERT_TRUE(m_db->TableExists("TestTable10"));
    ASSERT_TRUE(m_db->ColumnExists("TestTable10", "Column10"));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MergeSchemaChanges)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"SchemaChanges.bim");
    m_db->CreateTable("TestTable", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->SaveChanges("Created Initial Model");
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    /* Create revision with schema changes */
    ASSERT_EQ(m_db->AddColumnToTable("TestTable", "Column2", "INTEGER"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->CreateIndex("idx_TestTable_Column1", "TestTable", false, "Column1"), BE_SQLITE_OK);

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDbSchemaChanges());
   
    m_db->SaveChanges("Schema changes");
    DgnRevisionPtr schemaChangesRevision = CreateRevision();
    ASSERT_TRUE(schemaChangesRevision.IsValid());
    DumpRevision(*schemaChangesRevision, "Revision with schema changes:");

    /* Restore baseline, make data changes, and merge revision with schema changes */
    RestoreTestFile();
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable(Id, Column1) VALUES(1,1)"), BE_SQLITE_OK);
    
    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDbSchemaChanges());
    
    m_db->SaveChanges("Data changes");
    MergeSchemaRevision(*schemaChangesRevision);

    /* Create new revision with just the data changes */
    DgnRevisionPtr dataChangesRevision = CreateRevision();
    ASSERT_TRUE(dataChangesRevision.IsValid());
    DumpRevision(*dataChangesRevision, "Revision with data changes:");

    /* Create new revision with more data changes on top of the previous schema changes */
    ASSERT_EQ(m_db->ExecuteSql("UPDATE TestTable SET Column2=1 WHERE Id=1"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable(Id,Column1,Column2) VALUES(2,2,2)"), BE_SQLITE_OK);
   
    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDbSchemaChanges());

    m_db->SaveChanges("More data changes");
    DgnRevisionPtr moreDataChangesRevision = CreateRevision();
    ASSERT_TRUE(moreDataChangesRevision.IsValid());
    DumpRevision(*moreDataChangesRevision, "Revision with more data changes:");

    /* Restore baseline, and merge revisions previously created */
    RestoreTestFile();
    MergeSchemaRevision(*schemaChangesRevision);

    ASSERT_TRUE(m_db->ColumnExists("TestTable", "Column2"));

    RevisionStatus status = m_db->Revisions().MergeRevision(*dataChangesRevision);
    ASSERT_TRUE(status == RevisionStatus::Success);

    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable WHERE Id=1", 1));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable WHERE Id=1", 0)); // i.e., null value

    status = m_db->Revisions().MergeRevision(*moreDataChangesRevision);
    ASSERT_TRUE(status == RevisionStatus::Success);

    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable WHERE Id=1", 1));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable WHERE Id=2", 2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2017
//---------------------------------------------------------------------------------------
int GetColumnCount(DgnDb const& dgndb, Utf8CP tableName)
    {
    Statement statement;
    DbResult status = statement.TryPrepare(dgndb, SqlPrintfString("SELECT * FROM %s LIMIT 0", tableName));
    BeAssert(status == BE_SQLITE_OK);

    return statement.GetColumnCount();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, TableAndColumnAdditions)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"TableAndColumnAdditions.bim");
    DgnRevisionPtr revision0 = CreateRevision();
    ASSERT_TRUE(revision0.IsValid());
    BackupTestFile();

    // Get the BisCore.DefinitionElement
    ECSchemaCP bisCoreSchema = m_db->Schemas().GetSchema(Utf8String("BisCore"));
    ASSERT_TRUE(bisCoreSchema != nullptr);
    ECClassCP definitionElement = m_db->Schemas().GetClass(Utf8String("BisCore"), Utf8String("DefinitionElement"));
    ASSERT_TRUE(definitionElement != nullptr);

    // Create a schema with a sub-class of definition element that will cause a new overflow table. 
    ECSchemaPtr testSchema;
    ECObjectsStatus status = ECSchema::CreateSchema(testSchema, Utf8String("TableAndColumnAdditionTest"), Utf8String("tcat"), 1, 0, 0);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    testSchema->AddReferencedSchema(*(const_cast<ECSchemaP>(bisCoreSchema)));

    ECEntityClassP testClass = nullptr;
    status = testSchema->CreateEntityClass(testClass, "TestElement");
    ASSERT_TRUE(status == ECObjectsStatus::Success && testClass != nullptr);
    status = testClass->AddBaseClass(*definitionElement);
    ASSERT_TRUE(status == ECObjectsStatus::Success);

    for (int ii = 0; ii < 40; ii++)
        {
        Utf8PrintfString propName("Property%d", ii);
        PrimitiveECPropertyP prop = nullptr;
        status = testClass->CreatePrimitiveProperty(prop, propName);
        ASSERT_TRUE(status == ECObjectsStatus::Success);
        }

    int beforeCount = GetColumnCount(*m_db, "bis_DefinitionElement");
    ASSERT_FALSE(m_db->TableExists("bis_DefinitionElement_Overflow"));

    // Import schema with table/column additions, and create a revision with it
    bvector<ECSchemaCP> schemas;
    schemas.push_back(testSchema.get());
    SchemaStatus schemaStatus = m_db->ImportSchemas(schemas);
    ASSERT_TRUE(schemaStatus == SchemaStatus::Success);
    m_db->SaveChanges("Imported Test schema");

    DgnRevisionPtr revision1 = CreateRevision();
    ASSERT_TRUE(revision1.IsValid());
    DumpRevision(*revision1, "Revision with TestSchema import:");

    int afterCount = GetColumnCount(*m_db, "bis_DefinitionElement");
    ASSERT_TRUE(afterCount > beforeCount);
    ASSERT_TRUE(m_db->TableExists("bis_DefinitionElement_Overflow"));

    /* Restore baseline, make data changes, and merge revision with schema changes */
    RestoreTestFile();

    ASSERT_EQ(beforeCount, GetColumnCount(*m_db, "bis_DefinitionElement"));
    ASSERT_FALSE(m_db->TableExists("bis_DefinitionElement_Overflow"));

    MergeSchemaRevision(*revision1);
    
    ASSERT_EQ(afterCount, GetColumnCount(*m_db, "bis_DefinitionElement"));
    ASSERT_TRUE(m_db->TableExists("bis_DefinitionElement_Overflow"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2018
//---------------------------------------------------------------------------------------
bvector<DgnRevisionCP> filterRevisions(DgnRevisionPtr* revisionPtrs, int startIndex, int finishIndex)
    {
    bvector<DgnRevisionCP> filteredRevisions;
    int incOrDec = finishIndex > startIndex ? +1 : -1;
    for (int ii = startIndex; ii != finishIndex + incOrDec; ii += incOrDec)
        filteredRevisions.push_back(revisionPtrs[ii].get());
    return filteredRevisions;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2017
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MoreDataAndSchemaChanges)
    {
    /* These scenarios have been reported by the IModelHub team */
    DgnRevisionPtr revisionPtrs[11]; // 0 - Initial revision
    int ii = 0;

    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"MoreChanges.bim");
    m_db->CreateTable("TestTable1", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->SaveChanges("Created Initial Model");
    revisionPtrs[ii] = CreateRevision();
    ASSERT_TRUE(revisionPtrs[ii].IsValid());
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    BackupTestFile();

    // Create revisions 1-3 with data changes
    while (++ii <= 3)
        {
        Utf8PrintfString sql("INSERT INTO TestTable1(Id, Column1) VALUES(%d,%d)", ii, ii);
        ASSERT_EQ(m_db->ExecuteSql(sql.c_str()), BE_SQLITE_OK);
        m_db->SaveChanges();
        revisionPtrs[ii] = CreateRevision();
        ASSERT_TRUE(revisionPtrs[ii].IsValid());
        }

    // Create revision 4 with schema change
    m_db->CreateTable("TestTable2", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->SaveChanges();
    revisionPtrs[ii] = CreateRevision();
    ASSERT_TRUE(revisionPtrs[ii].IsValid());

    // Create revision 5-8 with data changes
    while (++ii <= 8)
        {
        Utf8PrintfString sql("INSERT INTO TestTable2(Id, Column1) VALUES(%d,%d)", ii, ii);
        ASSERT_EQ(m_db->ExecuteSql(sql.c_str()), BE_SQLITE_OK);
        m_db->SaveChanges();
        revisionPtrs[ii] = CreateRevision();
        ASSERT_TRUE(revisionPtrs[ii].IsValid());
        }

    // Create revision 9 with schema change
    m_db->CreateTable("TestTable3", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->SaveChanges();
    revisionPtrs[ii] = CreateRevision();
    ASSERT_TRUE(revisionPtrs[ii].IsValid());
    ++ii;

    // Create revision 10 with data change
    Utf8PrintfString sql("INSERT INTO TestTable3(Id, Column1) VALUES(%d,%d)", ii, ii);
    ASSERT_EQ(m_db->ExecuteSql(sql.c_str()), BE_SQLITE_OK);
    m_db->SaveChanges();
    revisionPtrs[ii] = CreateRevision();
    ASSERT_TRUE(revisionPtrs[ii].IsValid());
    ASSERT_TRUE(ii == 10);

    /* 
     * Test 1: Reopen with Reverse 
     */
    RestoreTestFile();
    DbResult openStatus;
    bvector<DgnRevisionCP> processRevisions;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes);

    // Merge Rev 1-6
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 1, 6);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Merge);
    m_db = DgnDb::OpenDgnDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    ASSERT_FALSE(m_db->Revisions().HasReversedRevisions());
    ASSERT_STREQ(m_db->Revisions().GetReversedRevisionId().c_str(), "");
    ASSERT_STREQ(m_db->Revisions().GetParentRevisionId().c_str(), revisionPtrs[6]->GetId().c_str());

    // Reverse to Rev 2 (i.e., 6-3)
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 6, 3);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Reverse);
    m_db = DgnDb::OpenDgnDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    ASSERT_TRUE(m_db->Revisions().HasReversedRevisions());
    ASSERT_STREQ(m_db->Revisions().GetReversedRevisionId().c_str(), revisionPtrs[2]->GetId().c_str());
    ASSERT_STREQ(m_db->Revisions().GetParentRevisionId().c_str(), revisionPtrs[6]->GetId().c_str());

    /*
    * Test 2: Invalid merge (of revision 7-9)
    */
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 7, 9);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Merge);
    BeTest::SetFailOnAssert(false);
    m_db = DgnDb::OpenDgnDb(&openStatus, fileName, openParams);
    BeTest::SetFailOnAssert(true);
    ASSERT_FALSE(m_db.IsValid()) << "Could perform an invalid merge";

    openParams.GetSchemaUpgradeOptionsR().Reset();
    m_db = DgnDb::OpenDgnDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";
    ASSERT_TRUE(m_db->Revisions().HasReversedRevisions());
    ASSERT_STREQ(m_db->Revisions().GetReversedRevisionId().c_str(), revisionPtrs[2]->GetId().c_str());
    ASSERT_STREQ(m_db->Revisions().GetParentRevisionId().c_str(), revisionPtrs[6]->GetId().c_str());

    /*
    * Test 3: Reinstate and merge
    */
    // Reinstate 3-6
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 3, 6);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Reinstate);
    m_db = DgnDb::OpenDgnDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    ASSERT_FALSE(m_db->Revisions().HasReversedRevisions());
    ASSERT_STREQ(m_db->Revisions().GetReversedRevisionId().c_str(), "");
    ASSERT_STREQ(m_db->Revisions().GetParentRevisionId().c_str(), revisionPtrs[6]->GetId().c_str());

    // Merge Rev 7-9
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 7, 9);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Merge);
    m_db = DgnDb::OpenDgnDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";
    
    ASSERT_FALSE(m_db->Revisions().HasReversedRevisions());
    ASSERT_STREQ(m_db->Revisions().GetReversedRevisionId().c_str(), "");
    ASSERT_STREQ(m_db->Revisions().GetParentRevisionId().c_str(), revisionPtrs[9]->GetId().c_str());
    }

#ifdef DEBUG_REVISION_TEST_MANUAL

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, TestMemoryLeak)
{
    BeFileName seedFile("D:\\temp\\Defects\\MemoryLeak\\ReadOnlyTest.bim", true);
    BeFileName copyFile = DgnDbTestDgnManager::GetOutputFilePath(L"ReadOnlyTest.bim");
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(seedFile.c_str(), copyFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

    Utf8CP changeSetIds[] =
    {
        "dbe4b3824129f99e4eb485fb7cd9d2fea2354be1",
        "20f94790e4a67782c2bafb93fa9b3955311c3fdb",
        "eb5075bd61a77c773b4a1e82c89087ba28b31aec",
        "1b186c485d182c46c02b99aff4fb12637263438f"
    };

    const int changeSetSize = sizeof(changeSetIds) / sizeof(Utf8CP);

    bvector<BeFileName> csPathnames;
    BeFileName basePath(L"D:\\temp\\Defects\\MemoryLeak\\csets\\");
    for (int ii = 0; ii < changeSetSize; ii++)
    {
        BeFileName csFileName(changeSetIds[ii], true);
        csFileName.AppendExtension(L"cs");

        BeFileName csPathname = basePath;
        csPathname.AppendToPath(csFileName);
        csPathnames.push_back(csPathname);
    }

    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite);
    m_db = DgnDb::OpenDgnDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    TestDataManager::MustBeBriefcase(m_db, Db::OpenMode::ReadWrite);

    Utf8String dbGuid = m_db->GetDbGuid().ToString();
    bvector<DgnRevisionPtr> revisionPtrs;
    bvector<DgnRevisionCP> revisions;

    Utf8String parentChangeSetId = m_db->Revisions().GetParentRevisionId();

    for (int ii = 0; ii < changeSetSize; ii++)
    {
        Utf8String changeSetId = changeSetIds[ii];
        if (ii > 0)
            parentChangeSetId = changeSetIds[ii - 1];

        DgnRevisionPtr rev = DgnRevision::Create(nullptr, changeSetId, parentChangeSetId, dbGuid);

        fileStatus = BeFileName::BeCopyFile(csPathnames[ii].c_str(), rev->GetRevisionChangesFile().c_str());
        ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

        //if (ii == 3)
        //    rev->Dump(*m_db);

        revisionPtrs.push_back(rev);
        revisions.push_back(rev.get());
    }

    m_db->CloseDb();

    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(revisions);
    openParams.SetStartDefaultTxn(DefaultTxn::No);

    printf("Before opening Db pre-upgrade");
    getchar();

    m_db = DgnDb::OpenDgnDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    m_db->CloseDb();
    m_db = nullptr;

    printf("After closing Db post-upgrade");
    getchar();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MergeMemoryIssue)
{
    BeFileName seedFile("D:\\temp\\Defects\\MemoryIssue\\119c3a62-d08a-4102-83e5-ff0902251740.bim", true);
    BeFileName copyFile = DgnDbTestDgnManager::GetOutputFilePath(L"119c3a62-d08a-4102-83e5-ff0902251740.bim");
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(seedFile.c_str(), copyFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

    Utf8CP changeSetIds[1] =
    {
        "d532b16cb2b21ca76d17170041df58df578044ec"
    };

    const int changeSetSize = sizeof(changeSetIds) / sizeof(Utf8CP);

    bvector<BeFileName> csPathnames;
    BeFileName basePath(L"D:\\temp\\Defects\\MemoryIssue\\");
    for (int ii = 0; ii < changeSetSize; ii++)
    {
        BeFileName csFileName(changeSetIds[ii], true);
        csFileName.AppendExtension(L"cs");

        BeFileName csPathname = basePath;
        csPathname.AppendToPath(csFileName);
        csPathnames.push_back(csPathname);
    }

    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite);
    m_db = DgnDb::OpenDgnDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    TestDataManager::MustBeBriefcase(m_db, Db::OpenMode::ReadWrite);

    Utf8String dbGuid = m_db->GetDbGuid().ToString();
    bvector<DgnRevisionPtr> revisionPtrs;
    bvector<DgnRevisionCP> revisions;

    Utf8String parentChangeSetId = m_db->Revisions().GetParentRevisionId();

    for (int ii = 0; ii < changeSetSize; ii++)
    {
        Utf8String changeSetId = changeSetIds[ii];
        if (ii > 0)
            parentChangeSetId = changeSetIds[ii - 1];

        DgnRevisionPtr rev = DgnRevision::Create(nullptr, changeSetId, parentChangeSetId, dbGuid);

        fileStatus = BeFileName::BeCopyFile(csPathnames[ii].c_str(), rev->GetRevisionChangesFile().c_str());
        ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

        //if (ii == 3)
        //    rev->Dump(*m_db);

        revisionPtrs.push_back(rev);
        revisions.push_back(rev.get());
    }

    m_db->CloseDb();

    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(revisions);
    openParams.SetStartDefaultTxn(DefaultTxn::No);

    printf("Before opening Db pre-upgrade");
    getchar();

    m_db = DgnDb::OpenDgnDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    m_db->CloseDb();
    m_db = nullptr;

    printf("After closing Db post-upgrade");
    getchar();
}

// Tests that are useful for one off testing and performance. These aren't included
// as part of the build, but used whenever necessary

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, DISABLED_MergeFolderWithRevisions)
    {
    BeFileName seedFile("D:\\temp\\Defects\\YII\\Oakland_west_Station.bim", true);
    BeFileName copyFile = DgnDbTestDgnManager::GetOutputFilePath(L"Oakland_west_Station.bim");
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(seedFile.c_str(), copyFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

    Utf8CP changeSetIds[4] =
        {
        "bd2644fc8fd0dc71d58b815e7dd805ba37b10641",
        "dffacfacf27056105c055e9ba573e0f45dcb7e10",
        "5113eff64bab7c12f04a30ec4f6da932751fd8d2",
        "5f1296bfaf45eb8da29dd00c10199795c64015d2"
        };

    bvector<BeFileName> csPathnames;
    BeFileName basePath(L"D:\\temp\\Defects\\YII\\");
    for (int ii = 0; ii < 4; ii++)
        {
        BeFileName csFileName(changeSetIds[ii], true);
        csFileName.AppendExtension(L"cs");

        BeFileName csPathname = basePath;
        csPathname.AppendToPath(csFileName);
        csPathnames.push_back(csPathname);
        }

    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite);
    m_db = DgnDb::OpenDgnDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    TestDataManager::MustBeBriefcase(m_db, Db::OpenMode::ReadWrite);

    Utf8String dbGuid = m_db->GetDbGuid().ToString();
    bvector<DgnRevisionPtr> revisionPtrs;
    bvector<DgnRevisionCP> revisions;
    for (int ii = 0; ii < 4; ii++)
        {
        Utf8String changeSetId = changeSetIds[ii];
        Utf8String parentChangeSetId = (ii > 0) ? changeSetIds[ii - 1] : "";

        DgnRevisionPtr rev = DgnRevision::Create(nullptr, changeSetId, parentChangeSetId, dbGuid);

        fileStatus = BeFileName::BeCopyFile(csPathnames[ii].c_str(), rev->GetRevisionChangesFile().c_str());
        ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

        //if (ii == 3)
        //    rev->Dump(*m_db);

        revisionPtrs.push_back(rev);
        revisions.push_back(rev.get());
        }

    m_db->CloseDb();

    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(revisions);
    m_db = DgnDb::OpenDgnDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, DISABLED_CreateAndMergePerformance)
    {
    // Setup a model with a few elements
    SetupDgnDb();
    m_db->SaveChanges("Created Initial Model");

    // Create an initial revision
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());

    Utf8String initialParentRevId = m_db->Revisions().GetParentRevisionId();

    StopWatch timer(false);
    double generateRevTime = 0.0;
    double mergeRevTime = 0.0;

    // Create and save multiple revisions
    BackupTestFile();
    bvector<DgnRevisionPtr> revisions;
    int dimension = 100;
    int numRevisions = 100;
    for (int revNum = 0; revNum < numRevisions; revNum++)
        {
        InsertFloor(dimension, dimension);
        m_db->SaveChanges("Inserted floor");

        timer.Start();
        DgnRevisionPtr revision = CreateRevision();
        timer.Stop();
        generateRevTime += timer.GetElapsedSeconds();

        ASSERT_TRUE(revision.IsValid());

        revisions.push_back(revision);
        }
    RestoreTestFile();

    // Dump all revisions
    for (DgnRevisionPtr const& rev : revisions)
        DumpRevision(*rev);

    // Merge all the saved revisions
    timer.Start();
    for (DgnRevisionPtr const& rev : revisions)
        {
        RevisionStatus status = m_db->Revisions().MergeRevision(*rev);
        ASSERT_TRUE(status == RevisionStatus::Success);
        }
    timer.Stop();
    mergeRevTime += timer.GetElapsedSeconds();

    // Check the updated revision id
    Utf8String mergedParentRevId = m_db->Revisions().GetParentRevisionId();
    ASSERT_TRUE(mergedParentRevId != initialParentRevId);

    // Abandon changes, and test that the parent revision and elements do not change
    m_db->AbandonChanges();

    Utf8String newParentRevId = m_db->Revisions().GetParentRevisionId();
    ASSERT_TRUE(newParentRevId == mergedParentRevId);

    LOG.infov("Time taken to generate revisions is %f", generateRevTime);
    LOG.infov("Time taken to merge revisions is %f", mergeRevTime);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, DISABLED_MergeFolderWithRevisions)
    {
    BeFileName startingFile("D:\\temp\\Performance\\Failure\\RevisionTestCopy.ibim", true);

    m_testFileName = DgnDbTestDgnManager::GetOutputFilePath(m_testFileName.c_str());
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(startingFile.c_str(), m_testFileName.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenDgnDb();

    bvector<BeFileName> revPathnames;
    BeFileListIterator fileList("D:\\temp\\Performance\\Failure\\DgnDbRev\\*.rev", false);
    BeFileName currFileName;
    while (SUCCESS == fileList.GetNextFileName(currFileName))
        revPathnames.push_back(currFileName);

    std::sort(revPathnames.begin(), revPathnames.end(), [] (BeFileNameCR a, BeFileNameCR b)
        {
        uint64_t aModTime = GetFileLastModifiedTime(a);
        uint64_t bModTime = GetFileLastModifiedTime(b);

        return aModTime < bModTime;
        });

    Utf8String dbGuid = m_db->GetDbGuid().ToString();
    for (BeFileNameCR revPathname : revPathnames)
        {
        Utf8String parentRevId = m_db->Revisions().GetParentRevisionId();
        Utf8String revId(BeFileName::GetFileNameWithoutExtension(revPathname.c_str()));
        DgnRevisionPtr rev = DgnRevision::Create(nullptr, revId, parentRevId, dbGuid);

        fileStatus = BeFileName::BeCopyFile(revPathname.c_str(), rev->GetChangeStreamFile().c_str());
        ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

        RevisionStatus status = m_db->Revisions().MergeRevision(*rev);

        if (status != RevisionStatus::Success)
            LOG.infov("Failed to merge revision: %s", revId.c_str());
        else
            LOG.infov("Success merging revision: %s", revId.c_str());

        ASSERT_TRUE(status == RevisionStatus::Success);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, DISABLED_MergeSpecificRevision)
    {
    BeFileName startingFile("D:\\temp\\Performance\\Failure\\RevisionTest.ibim", true);

    m_testFileName = DgnDbTestDgnManager::GetOutputFilePath(m_testFileName.c_str());
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(startingFile.c_str(), m_testFileName.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenDgnDb();

    BeFileName revPathname("D:\\temp\\Performance\\Failure\\DgnDbRev\\41469a8668091298800aae142eae402e6ac95842.rev", true); // 77th merge
    Utf8String parentRevId = m_db->Revisions().GetParentRevisionId();
    Utf8String revId(BeFileName::GetFileNameWithoutExtension(revPathname.c_str()));
    Utf8String dbGuid = m_db->GetDbGuid().ToString();
    DgnRevisionPtr rev = DgnRevision::Create(nullptr, revId, parentRevId, dbGuid);

    fileStatus = BeFileName::BeCopyFile(revPathname.c_str(), rev->GetChangeStreamFile().c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

    RevisionStatus status = m_db->Revisions().MergeRevision(*rev);

    if (status != RevisionStatus::Success)
        LOG.infov("Failed to merge revision: %s", revId.c_str());
    else
        LOG.infov("Success merging revision: %s", revId.c_str());

    ASSERT_TRUE(status == RevisionStatus::Success);
    }


#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
static TestElementCPtr insertTestElement(DgnModelR model, DgnCategoryId catid, int32_t intProp1)
    {
    DgnDbR db = model.GetDgnDb();
    auto el = TestElement::Create(db, model.GetModelId(), catid);
    el->SetIntegerProperty(0, intProp1);
    auto cpEl = db.Elements().Insert(*el);
    EXPECT_TRUE(cpEl.IsValid());
    return cpEl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void verifyIntegerProperty(DgnDbR db, DgnElementId eId, int32_t prop)
    {
    auto el = db.Elements().Get<TestElement>(eId);
    ASSERT_TRUE(el.IsValid());
    EXPECT_EQ(el->GetIntegerProperty(0), prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void updateIntegerProperty(DgnDbR db, DgnElementId eId, int32_t intProp1)
    {
    auto el = db.Elements().GetForEdit<TestElement>(eId);
    ASSERT_TRUE(el.IsValid());
    el->SetIntegerProperty(0, intProp1);
    ASSERT_TRUE(el->Update().IsValid());
    verifyIntegerProperty(db, eId, intProp1);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
static DgnRevisionPtr createRevision(DgnDbR db)
    {
    DgnRevisionPtr revision = db.Revisions().StartCreateRevision();
    if (!revision.IsValid())
        return nullptr;

    RevisionStatus status = db.Revisions().FinishCreateRevision();
    if (RevisionStatus::Success != status)
        {
        BeAssert(false);
        return nullptr;
        }

    return revision;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RevisionTestFixture, OptimisiticConcurrencyConflict)
    {
    // Get any profile upgrades out of the way. We don't want any of that to appear in the revision history for the purposes of this test.
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"upgraded.bim");
    m_db->SaveChanges();
    createRevision(*m_db);
    auto catId = m_defaultCategoryId;

    // Open two copies of the test bim.
    BeFileName firstDbName(m_db->GetFileName().GetDirectoryName());
    firstDbName.AppendToPath(L"first.bim");
    BeFileName::BeCopyFile(m_db->GetFileName(), firstDbName);
    auto first = DgnDb::OpenDgnDb(nullptr, firstDbName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    auto firstModel = first->Models().GetModel(m_defaultModel->GetModelId());

    BeFileName secondDbName(m_db->GetFileName().GetDirectoryName());
    secondDbName.AppendToPath(L"second.bim");
    BeFileName::BeCopyFile(m_db->GetFileName(), secondDbName);
    auto second = DgnDb::OpenDgnDb(nullptr, secondDbName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    // auto secondModel = second->Models().GetModel(m_defaultModel->GetModelId());

    // Configure both copies to use optimistic concurrency    
    OptimisticConcurrencyControl::Policy policy;
    policy.updateVsUpdate = OptimisticConcurrencyControl::OnConflict::RejectIncomingChange;
    policy.updateVsDelete = OptimisticConcurrencyControl::OnConflict::AcceptIncomingChange;
    policy.deleteVsUpdate = OptimisticConcurrencyControl::OnConflict::RejectIncomingChange;
    first->SetConcurrencyControl(new OptimisticConcurrencyControl(policy));  // adds ref
    second->SetConcurrencyControl(new OptimisticConcurrencyControl(policy)); // adds ref

    // The change "history" is a vector of DgnRevisions that we will be creating.
    bvector<DgnRevisionPtr> history;
    int firstParent = -1;
    int secondParent = -1;

    DgnElementId eid;   // The element that the two db's will fight over.
    DgnElementId eid2;  // another element that the two db's will fight over.
    DgnElementId eid3;  // another element that the two db's will fight over.
    
    // First: Create an element.
    int baseIntegerPropertyValue = 1;
    int expectedIntegerPropertyValueForElement2 = baseIntegerPropertyValue;
    int expectedIntegerPropertyValueForElement3 = baseIntegerPropertyValue;
    if (true) 
        {
        eid = insertTestElement(*firstModel, catId, baseIntegerPropertyValue)->GetElementId();
        eid2 = insertTestElement(*firstModel, catId, expectedIntegerPropertyValueForElement2)->GetElementId();
        eid3 = insertTestElement(*firstModel, catId, expectedIntegerPropertyValueForElement3)->GetElementId();
        first->SaveChanges();
        history.push_back(createRevision(*first));
        firstParent = 0;        // when I push a changeset, it automatically becomes my parent
        }

    // Second: Pull first's change (no conflict)
    if (true)
        {
        ASSERT_FALSE( second->Elements().Get<TestElement>(eid).IsValid() );
        ASSERT_FALSE( second->Elements().Get<TestElement>(eid2).IsValid() );
        ASSERT_EQ( RevisionStatus::Success, second->Revisions().MergeRevision(*history[++secondParent]) );
        ASSERT_TRUE( second->Elements().Get<TestElement>(eid).IsValid() );
        ASSERT_TRUE( second->Elements().Get<TestElement>(eid2).IsValid() );
        verifyIntegerProperty(*second, eid, baseIntegerPropertyValue);
        verifyIntegerProperty(*second, eid2, expectedIntegerPropertyValueForElement2);
        ASSERT_EQ(0, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsAccepted().size());
        ASSERT_EQ(0, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsRejected().size());
        }

    // ------------------------
    // --- update vs update ---
    // ------------------------
    int expectedIntegerPropertyValue = baseIntegerPropertyValue;

    // First: modify el
    int firstChangeValue;
    if (true)
        {
        verifyIntegerProperty(*first, eid, baseIntegerPropertyValue);
        verifyIntegerProperty(*second, eid2, expectedIntegerPropertyValueForElement2);
        updateIntegerProperty(*first, eid, firstChangeValue = 11);
        first->SaveChanges();
        history.push_back(createRevision(*first));
        ++firstParent;
        }

    // Second: modify el, then merge in first's change, and handle the conflict.
    if (true)
        {
        verifyIntegerProperty(*second, eid, baseIntegerPropertyValue);
        
        updateIntegerProperty(*second, eid, expectedIntegerPropertyValue = 22);
        second->SaveChanges();
        
        // merge first's changeset and reject his change
        ASSERT_EQ( RevisionStatus::Success, second->Revisions().MergeRevision(*history[++secondParent]) );

        ASSERT_EQ(0, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsAccepted().size());
        ASSERT_EQ(1, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsRejected().size());  // updateVsUpdate = RejectIncomingChange
        second->GetOptimisticConcurrencyControl()->_ConflictsProcessed();

        verifyIntegerProperty(*second, eid, expectedIntegerPropertyValue);
        verifyIntegerProperty(*second, eid2, expectedIntegerPropertyValueForElement2);

        history.push_back(createRevision(*second));
        ++secondParent;
        }

    // first: merge and see that second overrode my change. Verify that there is no conflict reported.
    if (true)
        {
        verifyIntegerProperty(*first, eid, firstChangeValue);
        ASSERT_EQ( RevisionStatus::Success, first->Revisions().MergeRevision(*history[++firstParent]) );
        verifyIntegerProperty(*first, eid, expectedIntegerPropertyValue);
        verifyIntegerProperty(*second, eid2, expectedIntegerPropertyValueForElement2);
        ASSERT_EQ(0, first->GetOptimisticConcurrencyControl()->_GetConflictingElementsAccepted().size());
        ASSERT_EQ(0, first->GetOptimisticConcurrencyControl()->_GetConflictingElementsRejected().size());
        }

    // -----------------------------------------------
    // --- Non-conflicting changes to same element ---
    // -----------------------------------------------

    // first: modify the property
    auto wasExpectedIntegerPropertyValue = expectedIntegerPropertyValue;
    if (true)
        {
        verifyIntegerProperty(*first, eid, expectedIntegerPropertyValue);
        updateIntegerProperty(*first, eid, expectedIntegerPropertyValue = 1111);
        first->SaveChanges();
        history.push_back(createRevision(*first));
        ++firstParent;
        }

    // second: modify userLabel
    auto expectedUserLabelValue = "foo";
    if (true)
        {
        auto el1before = second->Elements().GetElement(eid)->MakeCopy<TestElement>();
        el1before->SetUserLabel(expectedUserLabelValue);
        el1before->Update();
        second->SaveChanges();
        verifyIntegerProperty(*second, eid, wasExpectedIntegerPropertyValue);      // I didn't change the property

        // pull + merge => no conflict + both changes should be intact
        ASSERT_EQ( RevisionStatus::Success, second->Revisions().MergeRevision(*history[++secondParent]) );

        ASSERT_EQ(0, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsAccepted().size());
        ASSERT_EQ(1, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsRejected().size());  // NEEDS WORK: LastMod is reported as a conflict. While that's 
                                                                                                            // always indirect, first changed another column in bis_Element.
        second->GetOptimisticConcurrencyControl()->_ConflictsProcessed();                                   // So, the in-coming change, which includes LastMod and the other property, is marked as direct.

        auto el1after = second->Elements().Get<TestElement>(eid);
        verifyIntegerProperty(*second, eid, expectedIntegerPropertyValue);        // I see first's change to the property
        ASSERT_STREQ(expectedUserLabelValue, el1after->GetUserLabel());           // I continue to see my change to user label

        history.push_back(createRevision(*second));
        ++secondParent;
        }

    // first: pull and see both changes
    if (true)
        {
        ASSERT_EQ( RevisionStatus::Success, first->Revisions().MergeRevision(*history[++firstParent]) );

        ASSERT_EQ(0, first->GetOptimisticConcurrencyControl()->_GetConflictingElementsAccepted().size());
        ASSERT_EQ(0, first->GetOptimisticConcurrencyControl()->_GetConflictingElementsRejected().size());

        verifyIntegerProperty(*first, eid, expectedIntegerPropertyValue);
        auto el1after = first->Elements().Get<TestElement>(eid);
        ASSERT_STREQ(expectedUserLabelValue, el1after->GetUserLabel());
        }

    // ------------------------
    // --- delete vs update ---
    // ------------------------

    // First: modify el
    baseIntegerPropertyValue = expectedIntegerPropertyValue;
    if (true)
        {
        verifyIntegerProperty(*first, eid, baseIntegerPropertyValue);
        updateIntegerProperty(*first, eid, expectedIntegerPropertyValue = 111);
        first->SaveChanges();
        history.push_back(createRevision(*first));
        ++firstParent;
        }

    // Second: delete el, then merge in first's change, and handle the conflict in my favor.
    if (true)
        {
        verifyIntegerProperty(*second, eid, baseIntegerPropertyValue);
        
        auto el = second->Elements().GetElement(eid);
        ASSERT_TRUE(el.IsValid());
        el->Delete();
        el = nullptr;
        second->SaveChanges();
        
        // merge first's changeset and reject his change
        ASSERT_EQ( RevisionStatus::Success, second->Revisions().MergeRevision(*history[++secondParent]) );

        ASSERT_EQ(0, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsAccepted().size());
        ASSERT_EQ(1, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsRejected().size());  // deleteVsUpdate => RejectIncomingChange
        second->GetOptimisticConcurrencyControl()->_ConflictsProcessed();

        auto elafter = second->Elements().GetElement(eid);
        ASSERT_FALSE(elafter.IsValid()) << " second deleted el and resolved the delete vs update conflict by rejecting the update, so el should be gone.";
        
        verifyIntegerProperty(*second, eid2, expectedIntegerPropertyValueForElement2);  // Nothing should have happened to el2.

        history.push_back(createRevision(*second));
        ++secondParent;
        }

    // first: merge and see that second overrode my change. Verify that there is no conflict reported.
    if (true)
        {
        verifyIntegerProperty(*first, eid, expectedIntegerPropertyValue);    // element is in my briefcase
        ASSERT_EQ( RevisionStatus::Success, first->Revisions().MergeRevision(*history[++firstParent]) );
        auto elafter = second->Elements().GetElement(eid);
        ASSERT_FALSE(elafter.IsValid()) << " second deleted el and resolved the delete vs update conflict by rejecting the update, so el should be gone.";
        verifyIntegerProperty(*second, eid2, expectedIntegerPropertyValueForElement2);  // Nothing should have happened to el2.
        ASSERT_EQ(0, first->GetOptimisticConcurrencyControl()->_GetConflictingElementsAccepted().size());
        ASSERT_EQ(0, first->GetOptimisticConcurrencyControl()->_GetConflictingElementsRejected().size());
        first->GetOptimisticConcurrencyControl()->_ConflictsProcessed();
        }

    // -------------------------
    // --- update vs delete ----
    // -------------------------
    // Note that eid was deleted in the test above. Therefore, work with eid2
    
    if (true)
        {
        auto el2 = first->Elements().GetElement(eid2);
        el2->Delete();
        first->SaveChanges();
        history.push_back(createRevision(*first));
        ++firstParent;
        }

    if (true)
        {
        updateIntegerProperty(*second, eid2, expectedIntegerPropertyValueForElement2 = 222);
        second->SaveChanges();

        // merge first's changeset
        ASSERT_EQ( RevisionStatus::Success, second->Revisions().MergeRevision(*history[++secondParent]) );

        ASSERT_EQ(1, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsAccepted().size());  // updateVsDelete = acceptIncomingChanges
        ASSERT_EQ(0, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsRejected().size());  
        second->GetOptimisticConcurrencyControl()->_ConflictsProcessed();

        ASSERT_FALSE(second->Elements().GetElement(eid2).IsValid()) << "First deleted el. Second updated el and resolved the update vs delete conflict by accepting the deletion. So, el2 should be gone.";

        history.push_back(createRevision(*second));
        ++secondParent;
        }

    // first: pull and verify that delete still stands
    if (true)
        {
        ASSERT_TRUE(!first->Elements().GetElement(eid2).IsValid()) << "eid2 should be gone in first's briefcase";
        ASSERT_EQ( RevisionStatus::Success, first->Revisions().MergeRevision(*history[++firstParent]) );
        ASSERT_TRUE(!first->Elements().GetElement(eid2).IsValid()) << "eid2 should still be gone in first's briefcase";
        ASSERT_EQ(0, first->GetOptimisticConcurrencyControl()->_GetConflictingElementsAccepted().size());
        ASSERT_EQ(0, first->GetOptimisticConcurrencyControl()->_GetConflictingElementsRejected().size());
        first->GetOptimisticConcurrencyControl()->_ConflictsProcessed();
        }

#ifdef NOT_SUPPORTED_1u2d1r

    // ----------------------------------------------------
    // --- update vs delete  - reject in-coming change ----
    // ----------------------------------------------------
    // Note that eid and eid3 were deleted in the tests above. Therefore, work with eid3

    policy.updateVsDelete = OptimisticConcurrencyControl::OnConflict::RejectIncomingChange;
    first->SetConcurrencyControl(new OptimisticConcurrencyControl(policy));  // adds ref
    second->SetConcurrencyControl(new OptimisticConcurrencyControl(policy)); // adds ref

    if (true)
        {
        auto el3 = first->Elements().GetElement(eid3);
        el3->Delete();
        first->SaveChanges();
        ASSERT_TRUE(!first->Elements().GetElement(eid3).IsValid()) << "eid3 should be gone in first's briefcase";
        history.push_back(createRevision(*first));
        ++firstParent;
        }

    if (true)
        {
        verifyIntegerProperty(*second, eid3, expectedIntegerPropertyValueForElement3);
        updateIntegerProperty(*second, eid3, expectedIntegerPropertyValueForElement3 = 333);
        second->SaveChanges();

        // merge first's changeset
        ASSERT_EQ( RevisionStatus::Success, second->Revisions().MergeRevision(*history[++secondParent]) );

        ASSERT_EQ(0, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsAccepted().size());  // updateVsDelete = rejectIncomingChanges
        ASSERT_EQ(1, second->GetOptimisticConcurrencyControl()->_GetConflictingElementsRejected().size());
        second->GetOptimisticConcurrencyControl()->_ConflictsProcessed();

        verifyIntegerProperty(*second, eid3, expectedIntegerPropertyValueForElement3);  // eid3 should still be in my briefcase, with my change.

        history.push_back(createRevision(*second));
        ++secondParent;
        }

    // first: merge and see that second overrode my change. Verify that there is no conflict reported.
    if (true)
        {
        ASSERT_TRUE(!first->Elements().GetElement(eid3).IsValid()) << "eid3 should be gone in first's briefcase";
        ASSERT_EQ( RevisionStatus::Success, first->Revisions().MergeRevision(*history[++firstParent]) );
        verifyIntegerProperty(*first, eid3, expectedIntegerPropertyValueForElement3); // eid3 should be back again
        ASSERT_EQ(0, first->GetOptimisticConcurrencyControl()->_GetConflictingElementsAccepted().size());
        ASSERT_EQ(0, first->GetOptimisticConcurrencyControl()->_GetConflictingElementsRejected().size());
        first->GetOptimisticConcurrencyControl()->_ConflictsProcessed();
        }

#endif

    first->SaveChanges();
    first = nullptr;
    second->SaveChanges();
    second = nullptr;
    }