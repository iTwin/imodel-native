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

        void CreateDgnDb() { T_Super::CreateDgnDb(m_testFileName); }
        void OpenDgnDb() { T_Super::OpenDgnDb(m_testFileName); }
        void InsertFloor(int xmax, int ymax);
        void DumpRevision(DgnRevisionCR revision);
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
TEST_F(RevisionTestFixture, WorkflowTest)
    {
    int dimension = 1;
    int numRevisions = 5;

    // Create an initial revision
    CreateDgnDb();
    InsertModel();
    InsertCategory();
    InsertFloor(dimension, dimension);
    CreateDefaultView();
    UpdateDgnDbExtents();
    m_testDb->SaveChanges("Created Initial Model");

    DgnRevisionPtr revision = m_testDb->Revisions().StartCreateRevision();
    ASSERT_TRUE(revision.IsValid());
    BentleyStatus status = m_testDb->Revisions().FinishCreateRevision();
    ASSERT_TRUE(SUCCESS == status);
     
    CloseDgnDb();

    // Copy the Db
    BeFileName originalFile = DgnDbTestDgnManager::GetOutputFilePath(m_testFileName);
    BeFileName copyFile = DgnDbTestDgnManager::GetOutputFilePath(L"RevisionTestCopy.idgndb");

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(originalFile.c_str(), copyFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

    // Create and save multiple revisions
    OpenDgnDb();
    bvector<DgnRevisionPtr> revisions;
    for (int revNum = 0; revNum < numRevisions; revNum++)
        {
        InsertFloor(dimension, dimension);
        m_testDb->SaveChanges();

        DgnRevisionPtr revision = m_testDb->Revisions().StartCreateRevision();
        ASSERT_TRUE(revision.IsValid());

        status = m_testDb->Revisions().FinishCreateRevision();
        ASSERT_TRUE(SUCCESS == status);

        revisions.push_back(revision);
        }

    // Replace the original file with the unmodified copy
    CloseDgnDb();
    fileStatus = BeFileName::BeCopyFile(copyFile.c_str(), originalFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenDgnDb();

    // Dump all revisions
    for (DgnRevisionPtr const& rev : revisions)
        DumpRevision(*rev);

    // Merge all the saved revisions
    status = m_testDb->Revisions().MergeRevisions(revisions);
    ASSERT_TRUE(status == SUCCESS);
    m_testDb->SaveChanges();
    }
