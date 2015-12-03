/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/Revision_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE

// Turn this on for debugging.
// #define DUMP_REVISION 1

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
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    
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
    RevisionStatus status = m_testDb->Revisions().MergeRevisions(revisions);
    ASSERT_TRUE(status == RevisionStatus::Success);
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
    RevisionStatus status = m_testDb->Revisions().MergeRevisions(revisions);
    ASSERT_TRUE(status != RevisionStatus::Success);
    BeTest::SetFailOnAssert(true);
    }

