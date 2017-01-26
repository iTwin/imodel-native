/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/Revision_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"
#include <DgnPlatform/DgnChangeSummary.h>
#include <DgnPlatform/GenericDomain.h>
#include <array>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"

// #define DEBUG_REVISION_TEST_MANUAL 1
#ifdef DEBUG_REVISION_TEST_MANUAL 
#include <windows.h>
#endif

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

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
    void RestoreTestFile();

    void ExtractCodesFromRevision(DgnCodeSet& assigned, DgnCodeSet& discarded);

    static Utf8String CodeToString(DgnCode const& code) { return Utf8PrintfString("%s:%s\n", code.GetScope().c_str(), code.GetValueCP()); }
    static void ExpectCode(DgnCode const& code, DgnCodeSet const& codes) { EXPECT_FALSE(codes.end() == codes.find(code)) << CodeToString(code).c_str(); }
    static void ExpectCodes(DgnCodeSet const& exp, DgnCodeSet const& actual)
        {
        EXPECT_EQ(exp.size(), actual.size());
        for (auto const& code : exp)
            ExpectCode(code, actual);
        }

    static void DumpCode(DgnCode const& code) { printf("    %s\n", CodeToString(code).c_str()); }
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

    DgnElementCPtr InsertPhysicalElementByCode(DgnCode const& code)
        {
        DgnClassId classId = m_db->Domains().GetClassId(generic_ElementHandler::GenericPhysicalObjectHandler::GetHandler());
        GenericPhysicalObject elem(GenericPhysicalObject::CreateParams(*m_db, m_defaultModel->GetModelId(), classId, m_defaultCategoryId, Placement3d(), code, nullptr, DgnElementId()));
        return elem.Insert();
        }

    DgnElementCPtr RenameElement(DgnElementCR el, DgnCode const& code)
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
    DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(ChangeTestFixture::s_seedFileInfo.id, DgnPlatformSeedManager::SeedDbOptions(false, true));

    //  The group's seed file is essentially the same as the root seed file, but with a different relative path.
    //  Note that we must put our group seed file in a group-specific sub-directory
    RevisionTestFixture::s_seedFileInfo = rootSeedInfo;
    RevisionTestFixture::s_seedFileInfo.fileName.SetName(L"RevisionTestFixture/RevisionTestFixture.bim");

    DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(rootSeedInfo.fileName, RevisionTestFixture::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, DgnPlatformTestDomain::ImportSchema(*db));
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
void RevisionTestFixture::RestoreTestFile()
    {
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    CloseDgnDb();
    BeFileName originalFile = fileName;
    BeFileName copyFile(originalFile.GetDirectoryName()); //DgnDbTestDgnManager::GetOutputFilePath(m_copyTestFileName);
    copyFile.AppendToPath(m_copyTestFileName);

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(copyFile.c_str(), originalFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenDgnDb(fileName);
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

    // Create Revision 2 after deleting the same element
    DgnElementCPtr el = m_db->Elements().Get<DgnElement>(elementId);
    ASSERT_TRUE(el.IsValid());
    DgnDbStatus status = m_db->Elements().Delete(*el);
    ASSERT_TRUE(status == DgnDbStatus::Success);
    el = nullptr;
    m_db->SaveChanges("Deleted same element");

    DgnRevisionPtr revision2 = CreateRevision();
    ASSERT_TRUE(revision2.IsValid());

    // Create Revision 3 deleting the test model (the API causes Elements to get deleted)
    RestoreTestFile();
    ASSERT_TRUE(m_defaultModel.IsValid());
    status = m_defaultModel->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);
    m_defaultModel = nullptr;
    m_db->SaveChanges("Deleted model and contained elements");

    DgnRevisionPtr revision3 = CreateRevision();
    ASSERT_TRUE(revision3.IsValid());

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
    ASSERT_TRUE(revStatus == RevisionStatus::MergeError);
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
    expectedCodes.insert(ViewDefinition::CreateCode(db, "Default"));
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

    auto cpElX1 = InsertPhysicalElementByCode(codeSpec->CreateCode("X", "1")),
        cpElY2 = InsertPhysicalElementByCode(codeSpec->CreateCode("Y", "2")),
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
    EXPECT_TRUE(m_db->Memory().PurgeUntil(0));
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

    // Value of dependent's TestIntegerProperty2 == root's TestIntegerProperty1
    DgnElementId rootId = InsertElement(123)->GetElementId();
    DgnElementId depId = InsertElement(456)->GetElementId();
    TestElementDependency::Insert(*m_db, rootId, depId, 2);

    // Save initial state
    m_db->SaveChanges();
    ASSERT_TRUE(CreateRevision().IsValid());
    VerifyDependentProperties(depId, {456, 0, 123, 0});
    BackupTestFile();

    // Create a revision with some direct changes to the dependent property
    UpdateDependentProperty(depId, 2, 789);
    VerifyDependentProperties(depId, {456, 0, 789, 0});
    m_db->SaveChanges();
    VerifyDependentProperties(depId, {456, 0, 123, 0});

    DgnRevisionPtr directChangesRev = CreateRevision();
    ASSERT_TRUE(directChangesRev.IsValid());
    DumpRevision(*directChangesRev, "Direct Changes to Dependency:");

    // Create a revision with some indirect changes to the dependent property
    RestoreTestFile();
    UpdateRootProperty(654, rootId);
    m_db->SaveChanges("Revision");
    VerifyRootProperty(rootId, 654);
    VerifyDependentProperties(depId, {456, 0, 654, 0});

    DgnRevisionPtr indirectChangesRev = CreateRevision();
    ASSERT_TRUE(indirectChangesRev.IsValid());
    DumpRevision(*indirectChangesRev, "Indirect Changes to Dependency:");

    // Make direct changes, and merge the revision with indirect changes
    RestoreTestFile();
    UpdateDependentProperty(depId, 2, 789);
    m_db->SaveChanges();
    VerifyDependentProperties(depId, {456, 0, 123, 0});
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*indirectChangesRev));
    VerifyDependentProperties(depId, {456, 0, 654, 0});

    // Make indirect changes, and merge the revision with direct changes
    RestoreTestFile();
    UpdateRootProperty(654, rootId);
    m_db->SaveChanges("Revision");
    VerifyRootProperty(rootId, 654);
    VerifyDependentProperties(depId, {456, 0, 654, 0});
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*directChangesRev));
    VerifyDependentProperties(depId, {456, 0, 654, 0});
    }

#ifdef DEBUG_REVISION_TEST_MANUAL
// Tests that are useful for one off testing and performance. These aren't included
// as part of the build, but used whenever necessary

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, CreateAndMergePerformance)
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
uint64_t GetFileLastModifiedTime(BeFileNameCR pathname)
    {
    time_t mtime;
    if (BeFileNameStatus::Success != pathname.GetFileTime(nullptr, nullptr, &mtime, pathname.c_str()))
        {
        BeAssert(false);
        return 0;
        }

    return (uint64_t) mtime;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MergeFolderWithRevisions)
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
TEST_F(RevisionTestFixture, MergeSpecificRevision)
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
