/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/Revision_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"
#include <DgnPlatform/DgnChangeSummary.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE

// Turn this on for debugging.
// #define DUMP_REVISION 1

// #define DUMP_CODES

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct RevisionTestFixture : ChangeTestFixture
    {
    DEFINE_T_SUPER(ChangeTestFixture)
    private:
        int m_z = 0;

    protected:
        WCharCP m_testFileName = L"RevisionTest.idgndb";
        WCharCP m_copyTestFileName = L"RevisionTestCopy.idgndb";

        void CreateDgnDb(int dimension);
        void OpenDgnDb() { T_Super::OpenDgnDb(m_testFileName); }

        void InsertFloor(int xmax, int ymax);
        void ModifyElement(DgnElementId elementId);

        DgnRevisionPtr CreateRevision();
        void DumpRevision(DgnRevisionCR revision);

        void BackupTestFile();
        void RestoreTestFile();

        typedef AuthorityIssuedCodeSet CodeSet;
        typedef AuthorityIssuedCode Code;
        void ExtractCodesFromRevision(CodeSet& assigned, CodeSet& discarded);

        static Utf8String CodeToString(Code const& code) { return Utf8PrintfString("%s:%s\n", code.GetNamespace().c_str(), code.GetValueCP()); }
        static void ExpectCode(Code const& code, CodeSet const& codes) { EXPECT_FALSE(codes.end() == codes.find(code)) << CodeToString(code).c_str(); }
        static void ExpectCodes(CodeSet const& exp, CodeSet const& actual)
            {
            EXPECT_EQ(exp.size(), actual.size());
            for (auto const& code : exp)
                ExpectCode(code, actual);
            }

        static void DumpCode(Code const& code) { printf("    %s\n", CodeToString(code).c_str()); }
        static void DumpCodes(CodeSet const& codes, Utf8StringCR msg="Codes:")
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

        DgnElementCPtr InsertPhysicalElement(Code const& code)
            {
            DgnClassId classId = m_testDb->Domains().GetClassId(dgn_ElementHandler::Physical::GetHandler());
            PhysicalElement elem(PhysicalElement::CreateParams(*m_testDb, m_testModel->GetModelId(), classId, m_testCategoryId, Placement3d(), code, nullptr, DgnElementId()));
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
        virtual void SetUp() override {}
        virtual void TearDown() override {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void RevisionTestFixture::InsertFloor(int xmax, int ymax)
    {
    int z = m_z++;
    for (int x = 0; x < xmax; x++)
        for (int y = 0; y < ymax; y++)
            InsertElement(x, y, z);
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

    BentleyStatus status = m_testDb->Revisions().FinishCreateRevision();
    if (SUCCESS != status)
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
    BeFileName originalFile = DgnDbTestDgnManager::GetOutputFilePath(m_testFileName);
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
    BeFileName originalFile = DgnDbTestDgnManager::GetOutputFilePath(m_testFileName);
    BeFileName copyFile = DgnDbTestDgnManager::GetOutputFilePath(m_copyTestFileName);

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(copyFile.c_str(), originalFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenDgnDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void RevisionTestFixture::CreateDgnDb(int dimension)
    {
    T_Super::CreateDgnDb(m_testFileName);
    InsertModel();
    InsertCategory();
    InsertFloor(dimension, dimension);
    CreateDefaultView();
    UpdateDgnDbExtents();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, Workflow)
    {
    int dimension = 5;
    int numRevisions = 5;

    // Setup a model with a few elements
    CreateDgnDb(dimension);
    m_testDb->SaveChanges("Created Initial Model");

    // Create an initial revision
    ASSERT_TRUE(m_testDb->Revisions().CanCreateRevision());
    
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    
    ASSERT_FALSE(m_testDb->Revisions().CanCreateRevision());

    // Create and save multiple revisions
    BackupTestFile();
    bvector<DgnRevisionPtr> revisions;
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
    BentleyStatus status = m_testDb->Revisions().MergeRevisions(revisions);
    ASSERT_TRUE(status == SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, ConflictError)
    {
    // Setup a model with a few elements
    int dimension = 1;
    CreateDgnDb(dimension);
    DgnElementId elementId = InsertElement(1, 1, 1);
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
    bvector<DgnRevisionPtr> revisions;
    revisions.push_back(revision);
    BeTest::SetFailOnAssert(false);
    BentleyStatus status = m_testDb->Revisions().MergeRevisions(revisions);
    ASSERT_TRUE(status != SUCCESS);
    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RevisionTestFixture::ExtractCodesFromRevision(CodeSet& assigned, CodeSet& discarded)
    {
    m_testDb->SaveChanges();
    DgnRevisionPtr rev = m_testDb->Revisions().StartCreateRevision();
    BeAssert(rev.IsValid());

    ChangeStreamFileReader stream(rev->GetChangeStreamFile());
    DgnChangeSummary summary(*m_testDb);
    EXPECT_EQ(SUCCESS, summary.FromChangeSet(stream));
    summary.GetCodes(assigned, discarded);

    m_testDb->Revisions().FinishCreateRevision();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RevisionTestFixture, Codes)
    {
    // Creating the DgnDb allocates some codes (category, model, view...)
    CreateDgnDb(1);
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
    auto defaultCode = DgnAuthority::CreateDefaultCode();
    auto auth = NamespaceAuthority::CreateNamespaceAuthority("MyAuthority", db);
    EXPECT_EQ(DgnDbStatus::Success, auth->Insert());

    auto cpElX1 = InsertPhysicalElement(auth->CreateCode("X", "1")),
         cpElY2 = InsertPhysicalElement(auth->CreateCode("Y", "2")),
         cpUncoded = InsertPhysicalElement(defaultCode);

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
    auto cpNewElY2 = InsertPhysicalElement(codeY2);

    // The code that was set to empty should be seen as discarded; the code that replaced empty should be seen as new; the reused code should not appear.
    ExtractCodesFromRevision(createdCodes, discardedCodes);

    expectedCodes.clear();
    expectedCodes.insert(codeX1);
    ExpectCodes(expectedCodes, discardedCodes);

    expectedCodes.clear();
    expectedCodes.insert(cpUncoded->GetCode());
    ExpectCodes(expectedCodes, createdCodes);
    }

