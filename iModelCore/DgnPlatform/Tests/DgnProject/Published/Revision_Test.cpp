/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/Revision_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"
#include <DgnPlatform/DgnChangeSummary.h>
#include <DgnPlatform/GenericDomain.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

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
    WCharCP m_copyTestFileName = L"RevisionTestCopy.idgndb";

    virtual void _CreateDgnDb() override;

    void InsertFloor(int xmax, int ymax);
    void ModifyElement(DgnElementId elementId);

    DgnRevisionPtr CreateRevision();
    void DumpRevision(DgnRevisionCR revision);

    void BackupTestFile();
    void RestoreTestFile();

    typedef DgnCodeSet CodeSet;
    typedef DgnCode Code;
    void ExtractCodesFromRevision(DgnCodeSet& assigned, CodeSet& discarded);

    static Utf8String CodeToString(DgnCode const& code) { return Utf8PrintfString("%s:%s\n", code.GetNamespace().c_str(), code.GetValueCP()); }
    static void ExpectCode(DgnCode const& code, CodeSet const& codes) { EXPECT_FALSE(codes.end() == codes.find(code)) << CodeToString(code).c_str(); }
    static void ExpectCodes(DgnCodeSet const& exp, CodeSet const& actual)
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
        AnnotationTextStyle style(*m_testDb);
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
        DgnClassId classId = m_testDb->Domains().GetClassId(generic_ElementHandler::GenericPhysicalObjectHandler::GetHandler());
        GenericPhysicalObject elem(GenericPhysicalObject::CreateParams(*m_testDb, m_testModel->GetModelId(), classId, m_testCategoryId, Placement3d(), code, nullptr, DgnElementId()));
        return elem.Insert();
        }

    DgnElementCPtr RenameElement(DgnElementCR el, Code const& code)
        {
        auto pEl = el.CopyForEdit();
        EXPECT_EQ(DgnDbStatus::Success, pEl->SetCode(code));
        auto cpEl = pEl->Update();
        EXPECT_TRUE(cpEl.IsValid());
        return cpEl;
        }
public:
    RevisionTestFixture(WCharCP filename = L"RevisionTest.idgndb", bool wantTestDomain=false) : T_Super(filename, wantTestDomain) {}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2016
//---------------------------------------------------------------------------------------
void RevisionTestFixture::_CreateDgnDb()
    {
    T_Super::_CreateDgnDb();

    InsertFloor(1, 1);
    CreateDefaultView(m_testModel->GetModelId());
    UpdateDgnDbExtents();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void RevisionTestFixture::InsertFloor(int xmax, int ymax)
    {
    int z = m_z++;
    for (int x = 0; x < xmax; x++)
        for (int y = 0; y < ymax; y++)
            InsertPhysicalElement(*m_testModel, m_testCategoryId, x, y, z);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    08/2015
//---------------------------------------------------------------------------------------
void RevisionTestFixture::DumpRevision(DgnRevisionCR revision)
    {
#ifdef DUMP_REVISION
    printf("---------------------------------------------------------\n");
    revision.Dump(*m_testDb);
    printf("\n\n");
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void RevisionTestFixture::ModifyElement(DgnElementId elementId)
    {
    RefCountedPtr<PhysicalElement> testElement = m_testDb->Elements().GetForEdit<PhysicalElement>(elementId);
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
    DgnRevisionPtr revision = m_testDb->Revisions().StartCreateRevision();
    if (!revision.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    RevisionStatus status = m_testDb->Revisions().FinishCreateRevision();
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
    CloseDgnDb();
    BeFileName originalFile = DgnDbTestDgnManager::GetOutputFilePath(m_testFileName.c_str());
    BeFileName copyFile = DgnDbTestDgnManager::GetOutputFilePath(m_copyTestFileName);

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(originalFile.c_str(), copyFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenDgnDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void RevisionTestFixture::RestoreTestFile()
    {
    CloseDgnDb();
    BeFileName originalFile = DgnDbTestDgnManager::GetOutputFilePath(m_testFileName.c_str());
    BeFileName copyFile = DgnDbTestDgnManager::GetOutputFilePath(m_copyTestFileName);

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(copyFile.c_str(), originalFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenDgnDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, Workflow)
    {
    // Setup a model with a few elements
    CreateDgnDb();
    m_testDb->SaveChanges("Created Initial Model");

    m_testModel->FillModel();
    int initialElementCount = (int) m_testModel->GetElements().size();

    // Create an initial revision
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    
    Utf8String initialParentRevId = m_testDb->Revisions().GetParentRevisionId();

    // Create and save multiple revisions
    BackupTestFile();
    bvector<DgnRevisionPtr> revisions;
    int dimension = 5;
    int numRevisions = 5;
    for (int revNum = 0; revNum < numRevisions; revNum++)
        {
        InsertFloor(dimension, dimension);
        m_testDb->SaveChanges("Inserted floor");

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
        RevisionStatus status = m_testDb->Revisions().MergeRevision(*rev);
        ASSERT_TRUE(status == RevisionStatus::Success);
        }
 
    // Check the updated element count
    m_testModel->FillModel();
    int mergedElementCount = (int) m_testModel->GetElements().size();
    int expectedElementCount = dimension * dimension * numRevisions + initialElementCount;
    ASSERT_EQ(expectedElementCount, mergedElementCount);

    // Check the updated revision id
    Utf8String mergedParentRevId = m_testDb->Revisions().GetParentRevisionId();
    ASSERT_TRUE(mergedParentRevId != initialParentRevId);

    // Abandon changes, and test that the parent revision and elements do not change
    m_testDb->AbandonChanges();
    
    m_testModel->FillModel();
    int abandonedElementCount = (int) m_testModel->GetElements().size();
    ASSERT_TRUE(abandonedElementCount == mergedElementCount);

    Utf8String newParentRevId = m_testDb->Revisions().GetParentRevisionId();
    ASSERT_TRUE(newParentRevId == mergedParentRevId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, ConflictError)
    {
    // Setup a model with a few elements
    CreateDgnDb();
    DgnElementId elementId = InsertPhysicalElement(*m_testModel, m_testCategoryId, 1, 1, 1);
    m_testDb->SaveChanges("Created Initial Model");

    // Create an initial revision
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());

    // Create a revision for modifying the above element. 
    BackupTestFile();
    ModifyElement(elementId);
    m_testDb->SaveChanges("Modified the element in revision");

    DgnRevisionPtr revision = CreateRevision();
    ASSERT_TRUE(revision.IsValid());

    RestoreTestFile();

    // Modify the same element to generate an intentional conflict
    ModifyElement(elementId);
    m_testDb->SaveChanges("Modified the element");

    // Merge changes from revision
    BeTest::SetFailOnAssert(false);
    RevisionStatus status = m_testDb->Revisions().MergeRevision(*revision);
    ASSERT_TRUE(status != RevisionStatus::Success);
    BeTest::SetFailOnAssert(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2016
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MoreWorkflow)
    {
    // Setup baseline
    CreateDgnDb();
    m_testDb->SaveChanges("Created Initial Model");
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());

    // Create Revision 1 inserting an element into the test model
    BackupTestFile();
    DgnElementId elementId = InsertPhysicalElement(*m_testModel, m_testCategoryId, 2, 2, 2);
    ASSERT_TRUE(elementId.IsValid());
    m_testDb->SaveChanges("Inserted an element");

    DgnRevisionPtr revision1 = CreateRevision();
    ASSERT_TRUE(revision1.IsValid());

    // Create Revision 2 after deleting the same element
    DgnElementCPtr el = m_testDb->Elements().Get<DgnElement>(elementId);
    ASSERT_TRUE(el.IsValid());
    DgnDbStatus status = m_testDb->Elements().Delete(*el);
    ASSERT_TRUE(status == DgnDbStatus::Success);
    el = nullptr;
    m_testDb->SaveChanges("Deleted same element");

    DgnRevisionPtr revision2 = CreateRevision();
    ASSERT_TRUE(revision2.IsValid());

    // Create Revision 3 deleting the test model (the API causes Elements to get deleted)
    RestoreTestFile();
    ASSERT_TRUE(m_testModel.IsValid());
    status = m_testModel->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);
    m_testModel = nullptr;
    m_testDb->SaveChanges("Deleted model and contained elements");

    DgnRevisionPtr revision3 = CreateRevision();
    ASSERT_TRUE(revision3.IsValid());

    RevisionStatus revStatus;

    // Merge Rev1 first
    RestoreTestFile();
    ASSERT_TRUE(m_testModel.IsValid());
    revStatus = m_testDb->Revisions().MergeRevision(*revision1);
    ASSERT_TRUE(revStatus == RevisionStatus::Success);

    // Merge Rev2 next
    revStatus = m_testDb->Revisions().MergeRevision(*revision2);
    ASSERT_TRUE(revStatus == RevisionStatus::Success);

    // Merge Rev3 next - should fail since the parent does not match
    BeTest::SetFailOnAssert(false);
    revStatus = m_testDb->Revisions().MergeRevision(*revision3);
    BeTest::SetFailOnAssert(true);
    ASSERT_TRUE(revStatus == RevisionStatus::ParentMismatch);

    // Merge Rev3 first
    RestoreTestFile();
    ASSERT_TRUE(m_testModel.IsValid());
    revStatus = m_testDb->Revisions().MergeRevision(*revision3);
    ASSERT_TRUE(revStatus == RevisionStatus::Success);
    
    // Delete model and Merge Rev1 - should fail since the model does not exist
    RestoreTestFile();
    status = m_testModel->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);
    m_testModel = nullptr;
    m_testDb->SaveChanges("Deleted model and contained elements");

    BeTest::SetFailOnAssert(false);
    revStatus = m_testDb->Revisions().MergeRevision(*revision1);
    BeTest::SetFailOnAssert(true);
    ASSERT_TRUE(revStatus == RevisionStatus::MergeError);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionTestFixture::ExtractCodesFromRevision(DgnCodeSet& assigned, CodeSet& discarded)
    {
    m_testDb->SaveChanges();
    DgnRevisionPtr rev = m_testDb->Revisions().StartCreateRevision();
    BeAssert(rev.IsValid());

    assigned = rev->GetAssignedCodes();
    discarded = rev->GetDiscardedCodes();

    m_testDb->Revisions().FinishCreateRevision();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RevisionTestFixture, Codes)
    {
    // Creating the DgnDb allocates some codes (category, model, view...)
    CreateDgnDb();
    DgnDbR db = *m_testDb;

    DgnCategoryCPtr defaultCat = DgnCategory::QueryCategory(m_testCategoryId, db);
    DgnSubCategory subCat(DgnSubCategory::CreateParams(db, m_testCategoryId, "MySubCategory", DgnSubCategory::Appearance()));
    auto cpSubCat = subCat.Insert();
    EXPECT_TRUE(cpSubCat.IsValid());

    DgnModelPtr defaultModel = m_testModel;
    ASSERT_TRUE(defaultCat.IsValid());
    ASSERT_TRUE(defaultModel.IsValid());

    // Check that the new codes are all reported
    CodeSet createdCodes, discardedCodes;
    ExtractCodesFromRevision(createdCodes, discardedCodes);

    CodeSet expectedCodes;
    ExpectCodes(expectedCodes, discardedCodes);

    expectedCodes.insert(defaultModel->GetCode());
    expectedCodes.insert(defaultCat->GetCode());
    expectedCodes.insert(DgnSubCategory::QuerySubCategory(defaultCat->GetDefaultSubCategoryId(), db)->GetCode());
    expectedCodes.insert(subCat.GetCode());
    expectedCodes.insert(ViewDefinition::CreateCode("Default"));
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
    auto auth = NamespaceAuthority::CreateNamespaceAuthority("MyAuthority", db);
    EXPECT_EQ(DgnDbStatus::Success, auth->Insert());

    auto cpElX1 = InsertPhysicalElementByCode(auth->CreateCode("X", "1")),
        cpElY2 = InsertPhysicalElementByCode(auth->CreateCode("Y", "2")),
        cpUncoded = InsertPhysicalElementByCode(defaultCode);

    ExtractCodesFromRevision(createdCodes, discardedCodes);

    expectedCodes.clear();
    ExpectCodes(expectedCodes, discardedCodes);

    expectedCodes.insert(cpElX1->GetCode());
    expectedCodes.insert(cpElY2->GetCode());
    ExpectCodes(expectedCodes, createdCodes);

    // Set one code to empty, and one empty code to a non-empty code, and delete one coded element, and create a new element with the same code as the deleted element
    cpUncoded = RenameElement(*cpUncoded, auth->CreateCode("Z"));
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
    virtual void _OnRootChanged(DgnDbR, ECInstanceId, DgnElementId, DgnElementId) override;
    virtual void _ProcessDeletedDependency(DgnDbR, dgn_TxnTable::ElementDep::DepRelData const&) override { }

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
    auto root = db.Elements().Get<TestElement>(rootId);
    auto dep = db.Elements().GetForEdit<TestElement>(depId);
    ASSERT_TRUE(root.IsValid() && dep.IsValid());

    uint8_t index = GetIndex(db, relId);
    dep->SetIntegerProperty(index, root->GetIntegerProperty(0));
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

    DependencyRevisionTest() : T_Super(L"DependencyRevisionTest.idgndb", true) { }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementCPtr DependencyRevisionTest::InsertElement(int32_t intProp1)
    {
    DgnDbR db = m_testModel->GetDgnDb();
    auto el = TestElement::Create(db, m_testModelId, m_testCategoryId);
    el->SetIntegerProperty(0, intProp1);
    auto cpEl = db.Elements().Insert(*el);
    EXPECT_TRUE(cpEl.IsValid());
    return cpEl;
    }

/*---------------------------------------------------------------------------------**//**
* Ensure the dependency callback works as expected...
* @bsimethod                                                    Paul.Connelly   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DependencyRevisionTest, TestDependency)
    {
    CreateDgnDb();
    auto& db = *m_testDb;
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

    db.SaveChanges("Modify root properties");

    c = db.Elements().Get<TestElement>(cId);
    EXPECT_EQ(321, c->GetIntegerProperty(2));
    EXPECT_EQ(654, c->GetIntegerProperty(3));
    }

