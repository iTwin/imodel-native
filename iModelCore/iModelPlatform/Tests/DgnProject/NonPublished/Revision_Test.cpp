/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/Profiler.h>
#include "ChangeTestFixture.h"
#include <DgnPlatform/DgnChangeSummary.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/EntityIdsChangeGroup.h>
#include <array>
#include <tuple>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <ECDb/ChangedIdsIterator.h>
#include <ECDb/ChangeIterator.h>

// #define DEBUG_REVISION_TEST_MANUAL 1
#ifdef DEBUG_REVISION_TEST_MANUAL
#include <windows.h>
#endif

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_EC

#define LOG (NativeLogging::CategoryLogger("DgnCore"))

// Turn this on for debugging.
// #define DUMP_REVISION 1
// #define DUMP_CODES

void expectToThrow(std::function<void()> fn, Utf8CP msg) {
    BeTest::SetFailOnAssert(false);
    try {
        fn();
        ASSERT_TRUE(false);
    } catch (std::exception const& e) {
        EXPECT_STREQ(e.what(), msg);
    }
    BeTest::SetFailOnAssert(true);
}

void expectToNotThrow(std::function<void()> fn, Utf8CP msg)
    {
    BeTest::SetFailOnAssert(false);
    try {
        fn();
    } catch (std::exception const& e) {
        EXPECT_STREQ(e.what(), msg);
        ASSERT_TRUE(false) << "Expected no exception, but got one.";
    }
    BeTest::SetFailOnAssert(true);
    }   

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RevisionTestFixture : ChangeTestFixture
{
DEFINE_T_SUPER(ChangeTestFixture)
protected:
    int m_z = 0;
    WCharCP m_copyTestFileName = L"RevisionTestCopy.ibim";

    void InsertFloor(int xmax, int ymax);
    void ModifyElement(DgnElementId elementId);

    ChangesetPropsPtr CreateRevision(Utf8CP);
    void DumpRevision(ChangesetPropsCR revision, Utf8CP summary = nullptr);

    void BackupTestFile();
    void RestoreTestFile(Db::OpenMode openMode = Db::OpenMode::ReadWrite);

    void ExtractCodesFromRevision(DgnCodeSet& assigned, DgnCodeSet& discarded);
    void ProcessSchemaRevision(ChangesetPropsCR revision, RevisionProcessOption revisionProcessOption);
    void MergeSchemaRevision(ChangesetPropsCR revision) {
        EXPECT_EQ(ChangesetStatus::Success, m_db->Txns().PullMergeApply(revision));
    }

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
        AnnotationTextStyle style(m_db->GetDictionaryModel());
        style.SetName(name);
        auto pStyle = style.Insert();
        EXPECT_TRUE(pStyle.IsValid());
        return pStyle;
        }
    AnnotationTextStyleCPtr RenameTextStyle(AnnotationTextStyleCR style, Utf8CP newName)
        {
        auto pStyle = style.CreateCopy();
        pStyle->SetName(newName);
        pStyle->Update();
        auto cpStyle = pStyle->GetDgnDb().Elements().Get<AnnotationTextStyle>(pStyle->GetElementId());
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
        auto cpEl = pEl->UpdateAndGet();
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
// @bsimethod
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
    TestDataManager::SetAsStandaloneDb(db, Db::OpenMode::ReadWrite);

    m_defaultCodeSpecId = DgnDbTestUtils::InsertCodeSpec(*db, "TestCodeSpec");
    ASSERT_TRUE(m_defaultCodeSpecId.IsValid());

    db->SaveChanges();
    // Create a dummy revision to purge transaction table for the test
    ChangesetPropsPtr rev = db->Txns().StartCreateChangeset();
    BeAssert(rev.IsValid());
    db->Txns().FinishCreateChangeset(-1);

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
// @bsimethod
//---------------------------------------------------------------------------------------
void RevisionTestFixture::InsertFloor(int xmax, int ymax)
    {
    int z = 1;
    for (int x = 0; x < xmax; x++)
        for (int y = 0; y < ymax; y++)
            RevisionTestFixture::InsertPhysicalElement(*m_db, *m_defaultModel, m_defaultCategoryId, x, y, z);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RevisionTestFixture::DumpRevision(ChangesetPropsCR revision, Utf8CP summary)
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
// @bsimethod
//---------------------------------------------------------------------------------------
void RevisionTestFixture::ModifyElement(DgnElementId elementId)
    {
    RefCountedPtr<PhysicalElement> testElement = m_db->Elements().GetForEdit<PhysicalElement>(elementId);
    ASSERT_TRUE(testElement.IsValid());

    Placement3d newPlacement = testElement->GetPlacement();
    newPlacement.GetOriginR().x += 1.0;

    testElement->SetPlacement(newPlacement);

    DgnDbStatus dbStatus = testElement->Update();
    ASSERT_TRUE(dbStatus == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangesetPropsPtr RevisionTestFixture::CreateRevision(Utf8CP ext)
    {
    ChangesetPropsPtr revision = m_db->Txns().StartCreateChangeset(ext);
    if (!revision.IsValid())
        return nullptr;

    m_db->Txns().FinishCreateChangeset(-1, ext != nullptr);
    return revision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RevisionTestFixture::ProcessSchemaRevision(ChangesetPropsCR revision, RevisionProcessOption revisionProcessOption)
    {
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    CloseDgnDb();

    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(revision, revisionProcessOption));
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    m_defaultCodeSpec = m_db->CodeSpecs().GetCodeSpec(m_defaultCodeSpecId);
    ASSERT_TRUE(m_defaultCodeSpec.IsValid());

    m_defaultModel = m_db->Models().Get<PhysicalModel>(m_defaultModelId);
    ASSERT_TRUE(m_defaultModel.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
    OpenIModelDb(fileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
    OpenIModelDb(fileName, openMode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, Workflow)
    {
    // Setup a model with a few elements
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"Workflow.bim");
    m_db->SaveChanges("Created Initial Model");

    // Create an initial revision
    ChangesetPropsPtr initialRevision = CreateRevision("-cs1");
    ASSERT_TRUE(initialRevision.IsValid());

    Utf8String initialParentRevId = m_db->Txns().GetParentChangesetId();

    // Create and save multiple revisions
    BackupTestFile();
    bvector<ChangesetPropsPtr> revisions;
    int dimension = 5;
    int numRevisions = 5;
    for (int revNum = 0; revNum < numRevisions; revNum++)
        {
        InsertFloor(dimension, dimension);
        m_db->SaveChanges("Inserted floor");

        ChangesetPropsPtr revision = CreateRevision(Utf8PrintfString("-cst%d", revNum).c_str());
        ASSERT_TRUE(revision.IsValid());
        ASSERT_FALSE(revision->ContainsDdlChanges(*m_db));

        revisions.push_back(revision);
        }
    RestoreTestFile();

    // Dump all revisions
    for (ChangesetPropsPtr const& rev : revisions)
        DumpRevision(*rev);

    // Merge all the saved revisions
    for (ChangesetPropsPtr const& rev : revisions)
        {
        ChangesetStatus status = m_db->Txns().PullMergeApply(*rev);
        ASSERT_TRUE(status == ChangesetStatus::Success);
        }

    // Check the updated revision id
    Utf8String mergedParentRevId = m_db->Txns().GetParentChangesetId();
    ASSERT_TRUE(mergedParentRevId != initialParentRevId);

    // Abandon changes, and test that the parent revision and elements do not change
    m_db->AbandonChanges();

    Utf8String newParentRevId = m_db->Txns().GetParentChangesetId();
    ASSERT_TRUE(newParentRevId == mergedParentRevId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MoreWorkflow)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"MoreWorkflow.bim");
    m_db->SaveChanges("Created Initial Model");
    ChangesetPropsPtr initialRevision = CreateRevision("-cs1");
    ASSERT_TRUE(initialRevision.IsValid());

    // Create Revision 1 inserting an element into the test model
    BackupTestFile();
    DgnElementId elementId = RevisionTestFixture::InsertPhysicalElement(*m_db, *m_defaultModel, m_defaultCategoryId, 2, 2, 2);
    ASSERT_TRUE(elementId.IsValid());
    m_db->SaveChanges("Inserted an element");

    ChangesetPropsPtr revision1 = CreateRevision("-cs2");
    ASSERT_TRUE(revision1.IsValid());
    ASSERT_FALSE(revision1->ContainsDdlChanges(*m_db));

    // Create Revision 2 after deleting the same element
    DgnElementCPtr el = m_db->Elements().Get<DgnElement>(elementId);
    ASSERT_TRUE(el.IsValid());
    DgnDbStatus status = m_db->Elements().Delete(*el);
    ASSERT_TRUE(status == DgnDbStatus::Success);
    el = nullptr;
    m_db->SaveChanges("Deleted same element");

    ChangesetPropsPtr revision2 = CreateRevision("-cs3");
    ASSERT_TRUE(revision2.IsValid());
    ASSERT_FALSE(revision2->ContainsDdlChanges(*m_db));

    // Create Revision 3 deleting the test model (the API causes Elements to get deleted)
    RestoreTestFile();
    ASSERT_TRUE(m_defaultModel.IsValid());
    status = m_defaultModel->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);
    m_defaultModel = nullptr;
    m_db->SaveChanges("Deleted model and contained elements");

    ChangesetPropsPtr revision3 = CreateRevision("-cs4");
    ASSERT_TRUE(revision3.IsValid());
    ASSERT_FALSE(revision3->ContainsDdlChanges(*m_db));

    ChangesetStatus revStatus;

    // Merge Rev1 first
    RestoreTestFile();
    ASSERT_TRUE(m_defaultModel.IsValid());
    revStatus = m_db->Txns().PullMergeApply(*revision1);
    ASSERT_TRUE(revStatus == ChangesetStatus::Success);

    // Merge Rev2 next
    revStatus = m_db->Txns().PullMergeApply(*revision2);
    ASSERT_TRUE(revStatus == ChangesetStatus::Success);

    // Merge Rev3 next - should fail since the parent does not match
    expectToThrow([&]() { m_db->Txns().PullMergeApply(*revision3); }, "changeset out of order");

    // Merge Rev3 first
    RestoreTestFile();
    ASSERT_TRUE(m_defaultModel.IsValid());
    revStatus = m_db->Txns().PullMergeApply(*revision3);
    ASSERT_TRUE(revStatus == ChangesetStatus::Success);

    // Delete model and Merge Rev1 - should fail since the model does not exist
    RestoreTestFile();
    status = m_defaultModel->Delete();
    ASSERT_TRUE(status == DgnDbStatus::Success);
    m_defaultModel = nullptr;
     m_db->SaveChanges("Deleted model and contained elements");

    // In rebase following error would not happen
    // expectToThrow([&]() { m_db->Txns().PullMergeApply(*revision1); }, "Detected 1 foreign key conflicts in ChangeSet. Aborting merge.");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MergeToReadonlyBriefcase)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"ReadonlyBriefcase.bim");
    m_db->SaveChanges();
    ChangesetPropsPtr initialRevision = CreateRevision("-cs1"); // Clears Txn table
    BackupTestFile();

    // Create some revision
    DgnElementId elementId = RevisionTestFixture::InsertPhysicalElement(*m_db, *m_defaultModel, m_defaultCategoryId, 2, 2, 2);
    ASSERT_TRUE(elementId.IsValid());
    m_db->SaveChanges("Inserted an element");
    ChangesetPropsPtr revision1 = CreateRevision("-cs2");
    ASSERT_TRUE(revision1.IsValid());

    // Restore the master file again, and open in Readonly mode
    RestoreTestFile(Db::OpenMode::Readonly);

    // Merge revision that was previously created to create a checkpoint file
    expectToThrow([&]() { m_db->Txns().PullMergeApply(*revision1); }, "file is readonly");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, ResetIdSequencesAfterApply)
    {
    // Setup a test file
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"ResetElementIdSequenceAfterApply.bim");
    m_db->SaveChanges("Created Initial Model");

    ChangesetPropsPtr initialRevision = CreateRevision("-cs1");
    ASSERT_TRUE(initialRevision.IsValid());

    DgnElementId idBeforeInserts;
    DbResult result = m_db->GetElementIdSequence().GetNextValue(idBeforeInserts);
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Backup the test file
    BackupTestFile();

    // Make some element inserts, and create the baseline last sequence id (for later comparision)
    InsertFloor(1, 1);
    m_db->SaveChanges("Inserted floor");

    ChangesetPropsPtr revision = CreateRevision("-cs2");
    ASSERT_TRUE(revision.IsValid());

    DgnElementId idAfterInserts;
    result = m_db->GetElementIdSequence().GetNextValue(idAfterInserts);
    ASSERT_TRUE(result == BE_SQLITE_OK);
    ASSERT_GE(idAfterInserts, idBeforeInserts);

    // Restore baseline file, apply the change sets with the same element inserts, and validate the last sequence id
    RestoreTestFile();

    ChangesetStatus status = m_db->Txns().PullMergeApply(*revision);
    ASSERT_TRUE(status == ChangesetStatus::Success);

    DgnElementId idAfterMerge;
    result = m_db->GetElementIdSequence().GetNextValue(idAfterMerge);
    ASSERT_TRUE(result == BE_SQLITE_OK);
    ASSERT_EQ(idAfterMerge, idAfterInserts);
    }

//=======================================================================================
// @bsistruct
//=======================================================================================
struct DependencyRevisionTest : RevisionTestFixture
{
    DEFINE_T_SUPER(RevisionTestFixture);

    TestElementCPtr InsertElement(int32_t intProp1);
    void UpdateRootProperty(int32_t intProp1, DgnElementId eId);
    void VerifyRootProperty(DgnElementId, int32_t);
    void UpdateDependentProperty(DgnElementId eId, uint8_t index, int32_t value);
    void VerifyDependentProperty(DgnElementId eId, uint8_t index, int32_t value);
    void VerifyDependentProperties(DgnElementId, std::array<int32_t, 4> const&);

    DependencyRevisionTest() { }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyRevisionTest::UpdateRootProperty(int32_t intProp1, DgnElementId eId)
    {
    auto el = m_db->Elements().GetForEdit<TestElement>(eId);
    ASSERT_TRUE(el.IsValid());
    el->SetIntegerProperty(0, intProp1);
    ASSERT_EQ(DgnDbStatus::Success, el->Update());
    VerifyRootProperty(eId, intProp1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyRevisionTest::VerifyRootProperty(DgnElementId eId, int32_t prop)
    {
    auto el = m_db->Elements().Get<TestElement>(eId);
    ASSERT_TRUE(el.IsValid());
    EXPECT_EQ(el->GetIntegerProperty(0), prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyRevisionTest::UpdateDependentProperty(DgnElementId eId, uint8_t index, int32_t value)
    {
    auto el = m_db->Elements().GetForEdit<TestElement>(eId);
    ASSERT_TRUE(el.IsValid());
    el->SetIntegerProperty(index, value);
    ASSERT_EQ(DgnDbStatus::Success, el->Update());
    VerifyDependentProperty(eId, index, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyRevisionTest::VerifyDependentProperty(DgnElementId eId, uint8_t index, int32_t value)
    {
    auto el = m_db->Elements().Get<TestElement>(eId);
    ASSERT_TRUE(el.IsValid());
    EXPECT_EQ(el->GetIntegerProperty(index), value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, DdlChanges)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"SchemaChanges.bim");
    m_db->SaveChanges("Created Initial Model");
    ChangesetPropsPtr initialRevision = CreateRevision("-cs1");
    ASSERT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    // Create Revision 1 (Schema changes - creating two tables)
    m_db->CreateTable("TestTable1", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->CreateTable("TestTable2", "Id INTEGER PRIMARY KEY, Column1 INTEGER");

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDdlChanges());
    ASSERT_TRUE(m_db->Txns().HasChanges());

    m_db->SaveChanges("Revision 1");
    ChangesetPropsPtr revision1 = CreateRevision("-cs2");
    ASSERT_TRUE(revision1.IsValid());
    DumpRevision(*revision1, "Revision 1 with only schema changes:");

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDdlChanges());
    ASSERT_FALSE(m_db->Txns().HasChanges());

    // Create Revision 2 (Data changes - inserts to both tables)
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable1(Id, Column1) VALUES(1,1)"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable2(Id, Column1) VALUES(1,1)"), BE_SQLITE_OK);

    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDdlChanges());
    ASSERT_TRUE(m_db->Txns().HasChanges());

    m_db->SaveChanges("Revision 2");
    ChangesetPropsPtr revision2 = CreateRevision("-cs3");
    ASSERT_TRUE(revision2.IsValid());
    DumpRevision(*revision2, "Revision 2 with only data changes:");

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDdlChanges());
    ASSERT_FALSE(m_db->Txns().HasChanges());

    // Create Revision 3 (Schema changes to first table, and data changes to the second)
    ASSERT_EQ(m_db->AddColumnToTable("TestTable1", "Column2", "INTEGER"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->CreateIndex("idx_TestTable1_Column1", "TestTable1", false, "Column1"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable2(Id, Column1) VALUES(2,2)"), BE_SQLITE_OK);

    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDdlChanges());
    ASSERT_TRUE(m_db->Txns().HasChanges());

    m_db->SaveChanges("Revision 3");
    ChangesetPropsPtr revision3 = CreateRevision("-cs4");
    ASSERT_TRUE(revision3.IsValid());
    DumpRevision(*revision3, "Revision 3 with schema and data changes:");

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDdlChanges());
    ASSERT_FALSE(m_db->Txns().HasChanges());

    // Create Revision 4 (Data changes to the first table, and schema changes to the other)
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable1(Id, Column1, Column2) VALUES(2,2,2)"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->ExecuteSql("UPDATE TestTable1 SET Column2=1 WHERE Id=1"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->AddColumnToTable("TestTable2", "Column2", "INTEGER"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->CreateIndex("idx_TestTable2_Column1", "TestTable2", false, "Column1"), BE_SQLITE_OK);

    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDdlChanges());
    ASSERT_TRUE(m_db->Txns().HasChanges());

    m_db->SaveChanges("Revision 4");
    ChangesetPropsPtr revision4 = CreateRevision("-cs5");
    ASSERT_TRUE(revision4.IsValid());
    DumpRevision(*revision4, "Revision 4 with schema and data changes:");

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDdlChanges());
    ASSERT_FALSE(m_db->Txns().HasChanges());

    // Revert Db to initial state
    RestoreTestFile();

    // Merge revision 1 (Schema changes - creating two tables)
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
    EXPECT_EQ(ChangesetStatus::Success, m_db->Txns().PullMergeApply(*revision2));

    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable1 WHERE Id=1", 1));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable2 WHERE Id=1", 1));

    // Merge revision 3 (Schema changes to first table, and data changes to the second)
    MergeSchemaRevision(*revision3);

    ASSERT_TRUE(m_db->ColumnExists("TestTable1", "Column2"));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable1 WHERE Id=1", 0)); // i.e., null value

    ASSERT_FALSE(m_db->ColumnExists("TestTable2", "Column2"));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable2 WHERE Id=2", 2));

    // Merge revision 4 (Data changes to the first table, and schema changes to the other)
    LOG.infov("Merging Revision 4");
    MergeSchemaRevision(*revision4);

    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable1 WHERE Id=1", 1));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable1 WHERE Id=2", 2));

    ASSERT_TRUE(m_db->ColumnExists("TestTable2", "Column2"));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable2 WHERE Id=1", 0)); // i.e., null value
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable2 WHERE Id=2", 0)); // i.e., null value

    // Reverse revision 4 (Note that all data or schema changes are entirely skipped in this apply)
    expectToThrow([&]() { m_db->Txns().ReverseChangeset(*revision4); }, "Cannot reverse a changeset containing schema changes");

    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    CloseDgnDb();
    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(*revision4, RevisionProcessOption::Reverse));
    expectToThrow([&]() { m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams); }, "Cannot reverse a changeset containing schema changes");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, InvalidSchemaChanges)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"SchemaChanges.bim");
    m_db->SaveChanges("Created Initial Model");

    ASSERT_TRUE(BE_SQLITE_OK == m_db->CreateTable("TestTable", "Id INTEGER PRIMARY KEY, Column1 INTEGER"));
    m_db->SaveChanges("Created test table");
    ChangesetPropsPtr initialRevision = CreateRevision("-cs1");
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
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, ManySchemaChanges)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"SchemaChanges.bim");
    m_db->SaveChanges("Created Initial Model");
    ChangesetPropsPtr initialRevision = CreateRevision("-cs1");
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
    ChangesetPropsPtr revision = CreateRevision("-cs2");
    ASSERT_TRUE(revision.IsValid());
    DumpRevision(*revision, "Revision containing many schema changes:");

    // Revert Db to initial state
    RestoreTestFile();

    // Merge revision 1
    MergeSchemaRevision(*revision);

    ASSERT_TRUE(m_db->TableExists("TestTable10"));
    ASSERT_TRUE(m_db->ColumnExists("TestTable10", "Column10"));
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MergeSchemaChanges)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"SchemaChanges.bim");
    m_db->CreateTable("TestTable", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->SaveChanges("Created Initial Model");
    ChangesetPropsPtr initialRevision = CreateRevision("-cs1");
    ASSERT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    /* Create revision with schema changes */
    ASSERT_EQ(m_db->AddColumnToTable("TestTable", "Column2", "INTEGER"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->CreateIndex("idx_TestTable_Column1", "TestTable", false, "Column1"), BE_SQLITE_OK);

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDdlChanges());

    m_db->SaveChanges("Schema changes");
    ChangesetPropsPtr schemaChangesRevision = CreateRevision("-cs2");
    ASSERT_TRUE(schemaChangesRevision.IsValid());
    DumpRevision(*schemaChangesRevision, "Revision with schema changes:");

    /* Restore baseline, make data changes, and merge revision with schema changes */
    RestoreTestFile();
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable(Id, Column1) VALUES(1,1)"), BE_SQLITE_OK);

    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDdlChanges());

    m_db->SaveChanges("Data changes");
    MergeSchemaRevision(*schemaChangesRevision);

    /* Create new revision with just the data changes */
    ChangesetPropsPtr dataChangesRevision = CreateRevision("-cs3");
    ASSERT_TRUE(dataChangesRevision.IsValid());
    DumpRevision(*dataChangesRevision, "Revision with data changes:");

    /* Create new revision with more data changes on top of the previous schema changes */
    ASSERT_EQ(m_db->ExecuteSql("UPDATE TestTable SET Column2=1 WHERE Id=1"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable(Id,Column1,Column2) VALUES(2,2,2)"), BE_SQLITE_OK);

    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDdlChanges());

    m_db->SaveChanges("More data changes");
    ChangesetPropsPtr moreDataChangesRevision = CreateRevision("-cs4");
    ASSERT_TRUE(moreDataChangesRevision.IsValid());
    DumpRevision(*moreDataChangesRevision, "Revision with more data changes:");

    /* Restore baseline, and merge revisions previously created */
    RestoreTestFile();
    MergeSchemaRevision(*schemaChangesRevision);

    ASSERT_TRUE(m_db->ColumnExists("TestTable", "Column2"));

    ChangesetStatus status = m_db->Txns().PullMergeApply(*dataChangesRevision);
    ASSERT_TRUE(status == ChangesetStatus::Success);

    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column1 FROM TestTable WHERE Id=1", 1));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable WHERE Id=1", 0)); // i.e., null value

    status = m_db->Txns().PullMergeApply(*moreDataChangesRevision);
    ASSERT_TRUE(status == ChangesetStatus::Success);

    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable WHERE Id=1", 1));
    ASSERT_TRUE(ValidateValue(*m_db, "SELECT Column2 FROM TestTable WHERE Id=2", 2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, SchemaChangesAfterDataChanges)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"SchemaChangesAfterDataChanges.bim");
    m_db->CreateTable("TestTable", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->SaveChanges("Created Initial Model");
    ChangesetPropsPtr initialRevision = CreateRevision("-cs1");
    ASSERT_TRUE(initialRevision.IsValid());

    /* Create revision with schema changes */
    ASSERT_EQ(m_db->AddColumnToTable("TestTable", "Column2", "INTEGER"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->CreateIndex("idx_TestTable_Column1", "TestTable", false, "Column1"), BE_SQLITE_OK);

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDdlChanges());

    m_db->SaveChanges("Schema changes");
    ChangesetPropsPtr schemaChangesRevision = CreateRevision("-cs2");
    ASSERT_TRUE(schemaChangesRevision.IsValid());

    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable(Id,Column1,Column2) VALUES(1,1,1)"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable(Id,Column1,Column2) VALUES(2,2,2)"), BE_SQLITE_OK);

    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDdlChanges());

    m_db->SaveChanges("Data changes");

    ChangesetPropsPtr dataChangesRevision = CreateRevision("-cs3");
    ASSERT_TRUE(dataChangesRevision.IsValid());
    
    BackupTestFile();
    
    ASSERT_EQ(m_db->ExecuteDdl("DROP INDEX idx_TestTable_Column1"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->CreateIndex("idx_TestTable_Column2", "TestTable", false, "Column2"), BE_SQLITE_OK);

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDdlChanges());

    m_db->SaveChanges("Schema changes again");

    ChangesetPropsPtr secondSchemaChangesRevision = CreateRevision("-cs4");
    ASSERT_TRUE(secondSchemaChangesRevision.IsValid());

    /* Restore baseline, make data changes, and merge revision with schema changes */
    RestoreTestFile();

    MergeSchemaRevision(*secondSchemaChangesRevision);

    CloseDgnDb();
    
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int GetColumnCount(DgnDb const& dgndb, Utf8CP tableName)
    {
    Statement statement;
    DbResult status = statement.TryPrepare(dgndb, SqlPrintfString("SELECT * FROM %s LIMIT 0", tableName));
    BeAssert(status == BE_SQLITE_OK);

    return statement.GetColumnCount();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, TableAndColumnAdditions)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"TableAndColumnAdditions.bim");
    ChangesetPropsPtr revision0 = CreateRevision("-cs1");
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
        status = testClass->CreatePrimitiveProperty(prop, propName, PRIMITIVETYPE_String);
        ASSERT_TRUE(status == ECObjectsStatus::Success);
        }

    int beforeCount = GetColumnCount(*m_db, "bis_DefinitionElement");
    ASSERT_FALSE(m_db->TableExists("bis_DefinitionElement_Overflow"));

    // Import schema with table/column additions, and create a revision with it
    bvector<ECSchemaCP> schemas;
    schemas.push_back(testSchema.get());
    SchemaStatus schemaStatus = m_db->ImportSchemas(schemas, true);
    ASSERT_TRUE(schemaStatus == SchemaStatus::Success);
    m_db->SaveChanges("Imported Test schema");

    ChangesetPropsPtr revision1 = CreateRevision("-cs2");
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
// @bsimethod
//---------------------------------------------------------------------------------------
bvector<ChangesetPropsCP> filterRevisions(ChangesetPropsPtr* revisionPtrs, int startIndex, int finishIndex)
    {
    bvector<ChangesetPropsCP> filteredRevisions;
    int incOrDec = finishIndex > startIndex ? +1 : -1;
    for (int ii = startIndex; ii != finishIndex + incOrDec; ii += incOrDec)
        filteredRevisions.push_back(revisionPtrs[ii].get());
    return filteredRevisions;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, MoreDataAndSchemaChanges)
    {
    /* These scenarios have been reported by the IModelHub team */
    ChangesetPropsPtr revisionPtrs[11]; // 0 - Initial revision
    int ii = 0;

    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"MoreChanges.bim");
    m_db->CreateTable("TestTable1", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->SaveChanges("Created Initial Model");
    revisionPtrs[ii] = CreateRevision("-cs1");
    ASSERT_TRUE(revisionPtrs[ii].IsValid());
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    BackupTestFile();

    // Create revisions 1-3 with data changes
    while (++ii <= 3)
        {
        Utf8PrintfString sql("INSERT INTO TestTable1(Id, Column1) VALUES(%d,%d)", ii, ii);
        ASSERT_EQ(m_db->ExecuteSql(sql.c_str()), BE_SQLITE_OK);
        m_db->SaveChanges();
        revisionPtrs[ii] = CreateRevision(Utf8PrintfString("-csdata%d", ii).c_str());
        ASSERT_TRUE(revisionPtrs[ii].IsValid());
        }

    // Create revision 4 with schema change
    m_db->CreateTable("TestTable2", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->SaveChanges();
    revisionPtrs[ii] = CreateRevision("-cs3");
    ASSERT_TRUE(revisionPtrs[ii].IsValid());

    // Create revision 5-8 with data changes
    while (++ii <= 8)
        {
        Utf8PrintfString sql("INSERT INTO TestTable2(Id, Column1) VALUES(%d,%d)", ii, ii);
        ASSERT_EQ(m_db->ExecuteSql(sql.c_str()), BE_SQLITE_OK);
        m_db->SaveChanges();
        revisionPtrs[ii] = CreateRevision(Utf8PrintfString("-cs2data%d", ii).c_str());
        ASSERT_TRUE(revisionPtrs[ii].IsValid());
        }

    // Create revision 9 with schema change
    m_db->CreateTable("TestTable3", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->SaveChanges();
    revisionPtrs[ii] = CreateRevision("-cs5");
    ASSERT_TRUE(revisionPtrs[ii].IsValid());
    ++ii;

    // Create revision 10 with data change
    Utf8PrintfString sql("INSERT INTO TestTable3(Id, Column1) VALUES(%d,%d)", ii, ii);
    ASSERT_EQ(m_db->ExecuteSql(sql.c_str()), BE_SQLITE_OK);
    m_db->SaveChanges();
    revisionPtrs[ii] = CreateRevision("-cs6");
    ASSERT_TRUE(revisionPtrs[ii].IsValid());
    ASSERT_TRUE(ii == 10);

    /*
     * Test 1: Reopen with Reverse
     */
    RestoreTestFile();
    DbResult openStatus;
    bvector<ChangesetPropsCP> processRevisions;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes);

    // Merge Rev 1-6
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 1, 6);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Merge);
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    ASSERT_STREQ(m_db->Txns().GetParentChangesetId().c_str(), revisionPtrs[6]->GetChangesetId().c_str());

    // Reverse to Rev 4 (i.e., 6-5)
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 6, 5);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Reverse);
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    /*
    * Test 2: Invalid merge (of revision 7-9)
    */
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 7, 9);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Merge);
    expectToThrow([&]() { m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);}, "changeset out of order");

    openParams.GetSchemaUpgradeOptionsR().Reset();
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid());

    /*
    * Test 3: Reinstate and merge
    */
    // Reinstate 5-6
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 5, 6);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Merge);
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    ASSERT_STREQ(m_db->Txns().GetParentChangesetId().c_str(), revisionPtrs[6]->GetChangesetId().c_str());

    // Merge Rev 7-9
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 7, 9);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Merge);
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    ASSERT_STREQ(m_db->Txns().GetParentChangesetId().c_str(), revisionPtrs[9]->GetChangesetId().c_str());
    }

#ifdef DEBUG_REVISION_TEST_MANUAL

//---------------------------------------------------------------------------------------
// @bsimethod
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
    m_db = DgnDb::OpenIModelDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    TestDataManager::SetAsStandAlone(m_db, Db::OpenMode::ReadWrite);

    Utf8String dbGuid = m_db->GetDbGuid().ToString();
    bvector<ChangesetPropsPtr> revisionPtrs;
    bvector<ChangesetPropsCP> revisions;

    Utf8String parentChangeSetId = m_db->Txns().GetParentRevisionId();

    for (int ii = 0; ii < changeSetSize; ii++)
    {
        Utf8String changeSetId = changeSetIds[ii];
        if (ii > 0)
            parentChangeSetId = changeSetIds[ii - 1];

        ChangesetPropsPtr rev = DgnRevision::Create(nullptr, changeSetId, parentChangeSetId, dbGuid);

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

    m_db = DgnDb::OpenIModelDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    m_db->CloseDb();
    m_db = nullptr;

    printf("After closing Db post-upgrade");
    getchar();
}

//---------------------------------------------------------------------------------------
// @bsimethod
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
    m_db = DgnDb::OpenIModelDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    TestDataManager::SetAsStandAlone(m_db, Db::OpenMode::ReadWrite);

    Utf8String dbGuid = m_db->GetDbGuid().ToString();
    bvector<ChangesetPropsPtr> revisionPtrs;
    bvector<ChangesetPropsCP> revisions;

    Utf8String parentChangeSetId = m_db->Txns().GetParentRevisionId();

    for (int ii = 0; ii < changeSetSize; ii++)
    {
        Utf8String changeSetId = changeSetIds[ii];
        if (ii > 0)
            parentChangeSetId = changeSetIds[ii - 1];

        ChangesetPropsPtr rev = DgnRevision::Create(nullptr, changeSetId, parentChangeSetId, dbGuid);

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

    m_db = DgnDb::OpenIModelDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    m_db->CloseDb();
    m_db = nullptr;

    printf("After closing Db post-upgrade");
    getchar();
}

// Tests that are useful for one off testing and performance. These aren't included
// as part of the build, but used whenever necessary

//---------------------------------------------------------------------------------------
// @bsimethod
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
    m_db = DgnDb::OpenIModelDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    TestDataManager::SetAsStandAlone(m_db, Db::OpenMode::ReadWrite);

    Utf8String dbGuid = m_db->GetDbGuid().ToString();
    bvector<ChangesetPropsPtr> revisionPtrs;
    bvector<ChangesetPropsCP> revisions;
    for (int ii = 0; ii < 4; ii++)
        {
        Utf8String changeSetId = changeSetIds[ii];
        Utf8String parentChangeSetId = (ii > 0) ? changeSetIds[ii - 1] : "";

        ChangesetPropsPtr rev = DgnRevision::Create(nullptr, changeSetId, parentChangeSetId, dbGuid);

        fileStatus = BeFileName::BeCopyFile(csPathnames[ii].c_str(), rev->GetRevisionChangesFile().c_str());
        ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

        //if (ii == 3)
        //    rev->Dump(*m_db);

        revisionPtrs.push_back(rev);
        revisions.push_back(rev.get());
        }

    m_db->CloseDb();

    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(revisions);
    m_db = DgnDb::OpenIModelDb(&openStatus, copyFile, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, DISABLED_CreateAndMergePerformance)
    {
    // Setup a model with a few elements
    SetupDgnDb();
    m_db->SaveChanges("Created Initial Model");

    // Create an initial revision
    ChangesetPropsPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());

    Utf8String initialParentRevId = m_db->Txns().GetParentRevisionId();

    StopWatch timer(false);
    double generateRevTime = 0.0;
    double mergeRevTime = 0.0;

    // Create and save multiple revisions
    BackupTestFile();
    bvector<ChangesetPropsPtr> revisions;
    int dimension = 100;
    int numRevisions = 100;
    for (int revNum = 0; revNum < numRevisions; revNum++)
        {
        InsertFloor(dimension, dimension);
        m_db->SaveChanges("Inserted floor");

        timer.Start();
        ChangesetPropsPtr revision = CreateRevision();
        timer.Stop();
        generateRevTime += timer.GetElapsedSeconds();

        ASSERT_TRUE(revision.IsValid());

        revisions.push_back(revision);
        }
    RestoreTestFile();

    // Dump all revisions
    for (ChangesetPropsPtr const& rev : revisions)
        DumpRevision(*rev);

    // Merge all the saved revisions
    timer.Start();
    for (ChangesetPropsPtr const& rev : revisions)
        {
        RevisionStatus status = m_db->Txns().PullMergeApply(*rev);
        ASSERT_TRUE(status == RevisionStatus::Success);
        }
    timer.Stop();
    mergeRevTime += timer.GetElapsedSeconds();

    // Check the updated revision id
    Utf8String mergedParentRevId = m_db->Txns().GetParentRevisionId();
    ASSERT_TRUE(mergedParentRevId != initialParentRevId);

    // Abandon changes, and test that the parent revision and elements do not change
    m_db->AbandonChanges();

    Utf8String newParentRevId = m_db->Txns().GetParentRevisionId();
    ASSERT_TRUE(newParentRevId == mergedParentRevId);

    LOG.infov("Time taken to generate revisions is %f", generateRevTime);
    LOG.infov("Time taken to merge revisions is %f", mergeRevTime);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, DISABLED_MergeFolderWithRevisions)
    {
    BeFileName startingFile("D:\\temp\\Performance\\Failure\\RevisionTestCopy.ibim", true);

    m_testFileName = DgnDbTestDgnManager::GetOutputFilePath(m_testFileName.c_str());
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(startingFile.c_str(), m_testFileName.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenIModelDb();

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
        Utf8String parentRevId = m_db->Txns().GetParentRevisionId();
        Utf8String revId(BeFileName::GetFileNameWithoutExtension(revPathname.c_str()));
        ChangesetPropsPtr rev = DgnRevision::Create(nullptr, revId, parentRevId, dbGuid);

        fileStatus = BeFileName::BeCopyFile(revPathname.c_str(), rev->GetChangeStreamFile().c_str());
        ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

        RevisionStatus status = m_db->Txns().PullMergeApply(*rev);

        if (status != RevisionStatus::Success)
            LOG.infov("Failed to merge revision: %s", revId.c_str());
        else
            LOG.infov("Success merging revision: %s", revId.c_str());

        ASSERT_TRUE(status == RevisionStatus::Success);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, DISABLED_MergeSpecificRevision)
    {
    BeFileName startingFile("D:\\temp\\Performance\\Failure\\RevisionTest.ibim", true);

    m_testFileName = DgnDbTestDgnManager::GetOutputFilePath(m_testFileName.c_str());
    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(startingFile.c_str(), m_testFileName.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenIModelDb();

    BeFileName revPathname("D:\\temp\\Performance\\Failure\\DgnDbRev\\41469a8668091298800aae142eae402e6ac95842.rev", true); // 77th merge
    Utf8String parentRevId = m_db->Txns().GetParentRevisionId();
    Utf8String revId(BeFileName::GetFileNameWithoutExtension(revPathname.c_str()));
    Utf8String dbGuid = m_db->GetDbGuid().ToString();
    ChangesetPropsPtr rev = DgnRevision::Create(nullptr, revId, parentRevId, dbGuid);

    fileStatus = BeFileName::BeCopyFile(revPathname.c_str(), rev->GetChangeStreamFile().c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);

    RevisionStatus status = m_db->Txns().PullMergeApply(*rev);

    if (status != RevisionStatus::Success)
        LOG.infov("Failed to merge revision: %s", revId.c_str());
    else
        LOG.infov("Success merging revision: %s", revId.c_str());

    ASSERT_TRUE(status == RevisionStatus::Success);
    }


#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, IterateOverRedundantSchemaChange)
    {
    constexpr size_t revCount = 5;
    std::array<ChangesetPropsPtr, revCount> revisionPtrs;
    auto& initialRevision = revisionPtrs[0];

    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"RevisionChangesStreamLargeRead.bim");
    m_db->SaveChanges("Created Initial Model");
    initialRevision = CreateRevision("-cs1");
    ASSERT_TRUE(initialRevision.IsValid());
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    BackupTestFile();

    // Revision 1 (Import schema)
    // Get the BisCore.DefinitionElement
    ECSchemaCP bisCoreSchema = m_db->Schemas().GetSchema("BisCore");
    ASSERT_TRUE(bisCoreSchema != nullptr);
    ECClassCP definitionElement = m_db->Schemas().GetClass("BisCore", "DefinitionElement");
    ASSERT_TRUE(definitionElement != nullptr);

    // Create a schema with a sub-class of definition element that will cause a new overflow table.
    const auto schemaName = "TableAndColumnAdditionTest";
    ECSchemaPtr testSchema;
    ECObjectsStatus status = ECSchema::CreateSchema(testSchema, schemaName, "tcat", 1, 0, 0);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    testSchema->AddReferencedSchema(*(const_cast<ECSchemaP>(bisCoreSchema)));

    ECEntityClassP testClass = nullptr;
    status = testSchema->CreateEntityClass(testClass, "TestElement");
    ASSERT_TRUE(status == ECObjectsStatus::Success && testClass != nullptr);
    status = testClass->AddBaseClass(*definitionElement);
    ASSERT_TRUE(status == ECObjectsStatus::Success);

    for (int i = 0; i < 40; i++)
        {
        Utf8PrintfString propName("Property%d", i);
        PrimitiveECPropertyP prop = nullptr;
        status = testClass->CreatePrimitiveProperty(prop, propName, PRIMITIVETYPE_String);
        ASSERT_TRUE(status == ECObjectsStatus::Success);
        }

    int beforeCount = GetColumnCount(*m_db, "bis_DefinitionElement");
    ASSERT_FALSE(m_db->TableExists("bis_DefinitionElement_Overflow"));

    SchemaStatus schemaStatus = m_db->ImportSchemas(bvector<ECSchemaCP>{testSchema.get()}, true);
    ASSERT_TRUE(schemaStatus == SchemaStatus::Success);
    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Revision 1: Imported Test schema"));

    revisionPtrs[1] = CreateRevision("-cs2");
    ASSERT_TRUE(revisionPtrs[1].IsValid());

    int afterCount = GetColumnCount(*m_db, "bis_DefinitionElement");
    ASSERT_TRUE(afterCount > beforeCount);
    ASSERT_TRUE(m_db->TableExists("bis_DefinitionElement_Overflow"));


    // Revision 2 (Add elements from schema)
    auto testPartition = DefinitionPartition::CreateAndInsert(*m_db->Elements().GetRootSubject(), "testPartition");
    EXPECT_TRUE(testPartition.IsValid());
    auto testModel = DefinitionModel::CreateAndInsert(*testPartition);
    EXPECT_TRUE(testModel.IsValid());
    DefinitionElementPtr testElement = new DefinitionElement(DefinitionElement::CreateParams(
        *m_db,
        testModel->GetModelId(),
        m_db->Schemas().GetClassId(schemaName, "TestElement")
    ));
    auto persistedElement = m_db->Elements().Insert(*testElement);
    EXPECT_TRUE(persistedElement.IsValid());

    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Revision 2"));
    revisionPtrs[2] = CreateRevision("-cs3");
    ASSERT_TRUE(revisionPtrs[2].IsValid());

    // Revision 3 (Delete elements from schema)
    EXPECT_EQ(DgnDbStatus::Success, persistedElement->Delete());
    EXPECT_EQ(DgnDbStatus::Success, testModel->Delete());
    EXPECT_EQ(DgnDbStatus::Success, testPartition->Delete());

    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Revision 3"));
    revisionPtrs[3] = CreateRevision("-cs4");
    ASSERT_TRUE(revisionPtrs[3].IsValid());

    // Revision 4 (Delete schema)
    ASSERT_TRUE(m_db->DropSchema(schemaName).IsSuccess());

    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Revision 4"));
    revisionPtrs[4] = CreateRevision("-cs5");
    ASSERT_TRUE(revisionPtrs[4].IsValid());

    // now load the revisions into change iterators

    bvector<BeFileName> revisionFileNames;
    revisionFileNames.reserve(revisionPtrs.size() - 1);
    // ignore the initial revision since it will have a bunch of inserted stuff for setup purposes that we're not trying to iterate over
    std::transform(revisionPtrs.begin() + 1, revisionPtrs.end(), std::back_inserter(revisionFileNames), [](const auto& r){ return r->GetFileName(); });

    // ignore the initial revision since it will have a bunch of inserted stuff for setup purposes that we're not trying to iterate over
    //RevisionChangesFileReader reader1(revisionPtrs[0]->GetRevisionChangesFile(), *m_db);
    ChangesetFileReader reader2(revisionPtrs[1]->GetFileName(), m_db.get());
    ChangesetFileReader reader3(revisionPtrs[2]->GetFileName(), m_db.get());
    ChangesetFileReader reader4(revisionPtrs[3]->GetFileName(), m_db.get());
    ChangesetFileReader reader5(revisionPtrs[4]->GetFileName(), m_db.get());
    // this pointer array compensates for a lack of copy or move constructors which make this type difficult to put in STL containers
    std::array<ChangesetFileReader*, revisionPtrs.size() - 1> revisionReaders { &reader2, &reader3, &reader4, &reader5  };

    ChangeGroup revisionsChangeGroup;
    for (const auto& reader : revisionReaders) reader->AddToChangeGroup(revisionsChangeGroup);
    ChangeSet aggregateChangeset;
    aggregateChangeset.FromChangeGroup(revisionsChangeGroup);

    ChangeIterator changeIterator(*m_db, aggregateChangeset);
    ChangedIdsIterator changedIdsIterator(*m_db, aggregateChangeset);
    EntityIdsChangeGroup entityIdsChangeGroup;
    entityIdsChangeGroup.ExtractChangedInstanceIdsFromChangeSets(*m_db, revisionFileNames);

    const auto dbOpToStr = [](DbOpcode op) { return op == DbOpcode::Delete ? "D" : op == DbOpcode::Update ? "U" : "I"; };

#define DONT_EXPECT() EXPECT_TRUE(false)

    // although we're trying to make sure that the changesets are collapsed, in actuality the process of inserting and
    // removing elements updates the LastMod on the model into which they were inserted (the repo model in this case)
    // so there will be that change, we just ignore it

    for (const auto& entry : changeIterator)
        {
        if (!entry.IsMapped()) continue; // don't care about non-ec changes (except fonts...)
        for (const auto& column : entry.MakePrimaryColumnIterator())
            // column iterator iterates over fields that there aren't changes for, so those changes are invalid
            // and can be considered "not changed"
            if (column.GetValue(Changes::Change::Stage::Old).IsValid())
                {
                const bool isRepoModelLastModChange
                    = entry.GetPrimaryInstanceId() == m_db->GetRepositoryModel()->GetModelId()
                    && column.GetPropertyAccessString() == "LastMod";
                EXPECT_TRUE(isRepoModelLastModChange)
                    << "in entry: " << entry.ToString() << "\n"
                    << "column changed, (prop:" << column.GetPropertyAccessString()
                    << "), from '" << column.GetValue(Changes::Change::Stage::Old).Format(4)
                    << "' to '" << column.GetValue(Changes::Change::Stage::New).Format(4) << "'";
                }
        }

    for (const auto& entry : changedIdsIterator)
        {
        if (!entry.IsMapped()) continue; // don't care about non-ec changes (except fonts...)
        const bool isRepoModelLastModChange = entry.GetPrimaryInstanceId() == m_db->GetRepositoryModel()->GetModelId();
        EXPECT_TRUE(isRepoModelLastModChange)
            << "entry was found where expected: (" << "Table:" << entry.GetTableName() << ", id:" << entry.GetPrimaryInstanceId().ToHexStr()
            << ", Op:" << dbOpToStr(entry.GetDbOpcode()) << ", PrimaryTable:" << (entry.IsPrimaryTable() ? "Y" : "N")
            << ")";
        }

    for (const auto& entry : entityIdsChangeGroup.aspectOps)
        DONT_EXPECT() << "aspect:" << dbOpToStr(entry.second) << " " << entry.first.ToHexStr() << "";
    for (const auto& entry : entityIdsChangeGroup.codeSpecOps)
        DONT_EXPECT() << "codeSpec:" << dbOpToStr(entry.second) << " " << entry.first.ToHexStr() << "";
    for (const auto& entry : entityIdsChangeGroup.elementOps)
        DONT_EXPECT() << "element:" << dbOpToStr(entry.second) << " " << entry.first.ToHexStr() << "";
    for (const auto& entry : entityIdsChangeGroup.fontOps)
        DONT_EXPECT() << "font:" << dbOpToStr(entry.second) << " " << entry.first.ToHexStr() << "";
    for (const auto& entry : entityIdsChangeGroup.modelOps)
        {
        const bool isRepoModelLastModChange = entry.first == m_db->GetRepositoryModel()->GetModelId();
        EXPECT_TRUE(isRepoModelLastModChange) << "model:" << dbOpToStr(entry.second) << " " << entry.first.ToHexStr() << "";
        }
    for (const auto& entry : entityIdsChangeGroup.relationshipOps)
        DONT_EXPECT() << "relationship:" << dbOpToStr(entry.second) << " " << entry.first.ToHexStr() << "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, CheckProfileVersionUpdateAfterMerge)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"CheckProfileVersionUpdateAfterMerge.bim");
    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Initialized db"));
    ChangesetPropsPtr initialRevision = CreateRevision("-initialize");
    EXPECT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);

    auto initialDgnDbVersion = m_db->GetProfileVersion();
    auto initialECDbVersion = m_db->GetECDbProfileVersion();

    {
    DbResult result = m_db->ExecuteSql("UPDATE be_Prop SET StrData = '{\"major\":99,\"minor\":98,\"sub1\":97,\"sub2\":96}' WHERE Namespace = 'ec_Db' and Name = 'SchemaVersion'");
    EXPECT_EQ(result, DbResult::BE_SQLITE_OK);
    }
    {
    DbResult result = m_db->ExecuteSql("UPDATE be_Prop SET StrData = '{\"major\":89,\"minor\":88,\"sub1\":87,\"sub2\":86}' WHERE Namespace = 'dgn_Db' and Name = 'SchemaVersion'");
    EXPECT_EQ(result, DbResult::BE_SQLITE_OK);
    }

    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Revision 1: updates profile version"));

    ChangesetPropsPtr revision = CreateRevision("-profileUpgrade");
    EXPECT_TRUE(revision.IsValid());

    RestoreTestFile();

    {
    auto dgnDbVersion = m_db->GetProfileVersion();
    auto ecDbVersion = m_db->GetECDbProfileVersion();
    EXPECT_EQ(0, dgnDbVersion.CompareTo(initialDgnDbVersion));
    EXPECT_EQ(0, ecDbVersion.CompareTo(initialECDbVersion));
    }

    EXPECT_EQ(ChangesetStatus::Success, m_db->Txns().PullMergeApply(*revision));

    {
    auto dgnDbVersion = m_db->GetProfileVersion();
    auto ecDbVersion = m_db->GetECDbProfileVersion();
    EXPECT_EQ(89, dgnDbVersion.GetMajor());
    EXPECT_EQ(88, dgnDbVersion.GetMinor());
    EXPECT_EQ(87, dgnDbVersion.GetSub1());
    EXPECT_EQ(86, dgnDbVersion.GetSub2());
    EXPECT_EQ(99, ecDbVersion.GetMajor());
    EXPECT_EQ(98, ecDbVersion.GetMinor());
    EXPECT_EQ(97, ecDbVersion.GetSub1());
    EXPECT_EQ(96, ecDbVersion.GetSub2());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, BrokenECDbProfileInRevision)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"BrokenECDbProfileInRevision.bim");
    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Initialized db"));
    ChangesetPropsPtr initialRevision = CreateRevision("-initialize");
    EXPECT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);

    {
    DbResult result = m_db->ExecuteSql("UPDATE be_Prop SET StrData = '____no___valid___json' WHERE Namespace = 'ec_Db' and Name = 'SchemaVersion'");
    EXPECT_EQ(result, DbResult::BE_SQLITE_OK);
    }

    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Revision 1: updates profile version"));

    ChangesetPropsPtr revision = CreateRevision("-profileUpgrade");
    EXPECT_TRUE(revision.IsValid());

    RestoreTestFile();

    expectToThrow([&]() { m_db->Txns().PullMergeApply(*revision); }, "failed to apply changes");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, BrokenDgnDbProfileInRevision)
    {
    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"BrokenDgnDbProfileInRevision.bim");
    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Initialized db"));
    ChangesetPropsPtr initialRevision = CreateRevision("-initialize");
    EXPECT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);

    {
    DbResult result = m_db->ExecuteSql("UPDATE be_Prop SET StrData = '____no___valid___json' WHERE Namespace = 'dgn_Db' and Name = 'SchemaVersion'");
    EXPECT_EQ(result, DbResult::BE_SQLITE_OK);
    }

    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Revision 1: updates profile version"));

    ChangesetPropsPtr revision = CreateRevision("-profileUpgrade");
    EXPECT_TRUE(revision.IsValid());

    RestoreTestFile();

    expectToThrow([&]() { m_db->Txns().PullMergeApply(*revision); }, "failed to apply changes");
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, DeleteClassConstraintViolationInCacheTable)
    {
    auto checkTestClassExists = [&](const bool shouldExist)
        {
        // Check class existence in schema and cache table in one place
        ASSERT_EQ(shouldExist, nullptr != m_db->Schemas().GetClass("TestSchema", "TestClass"));

        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db,
            "SELECT 1 FROM ec_cache_ClassHierarchy ch "
            "JOIN ec_Class c ON ch.classId = c.Id WHERE c.Name = 'TestClass'"));
        ASSERT_EQ(shouldExist, stmt.Step() == BE_SQLITE_ROW);
        stmt.Finalize();
        };

    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"DeleteClassConstraintViolationInCacheTable.bim");
    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Initialized db"));

    auto context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(m_db->GetSchemaLocater());

    BeFileName searchDirs[2];
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDirs[0]);
    searchDirs[0].AppendToPath(L"ECSchemas");
    searchDirs[1] = searchDirs[0];

    context->AddFirstSchemaPaths({ searchDirs[0].AppendToPath(L"Dgn"), searchDirs[1].AppendToPath(L"Standard") });

    // Set up a base dynamic schema with a class
    const auto schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="1.0.0" alias="CoreCA" />

            <ECCustomAttributes>
                <DynamicSchema xmlns = 'CoreCustomAttributes.1.0.0' />
            </ECCustomAttributes>

            <ECEntityClass typeName="TestClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaPtr initialSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(initialSchema, schemaXml, *context));
    m_db->ImportSchemas({ initialSchema.get() }, true);
    m_db->SaveChanges("Created Test Schema");

    // Create a revision and a backup point
    const auto initialRevision = CreateRevision("-initialize");
    ASSERT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    // Check if class exists
    checkTestClassExists(true);

    // Create a changeset to delete the class
    // Perform a major schema update that deletes the class
    const auto updatedSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="1.0.0" alias="CoreCA" />

            <ECCustomAttributes>
                <DynamicSchema xmlns = 'CoreCustomAttributes.1.0.0' />
            </ECCustomAttributes>
        </ECSchema>)xml";

    ECSchemaPtr updatedSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(updatedSchema, updatedSchemaXml, *context));
    m_db->ImportSchemas({ updatedSchema.get() }, true);
    m_db->SaveChanges("Updated Test Schema");

    const auto updatedRevision = CreateRevision("-schemaMajorUpdate");
    ASSERT_TRUE(updatedRevision.IsValid());

    // Class should be deleted
    checkTestClassExists(false);

    // Now that we have a changeset with a major schema update that deletes the class, restore the imodel to the backup state
    RestoreTestFile();

    // Make sure class still exists
    checkTestClassExists(true);

    // Apply the changeset with the major schema update that deletes the class
    // This should not throw an exception, but rather apply the changes successfully
    // and the class should be deleted from the cache table
    expectToNotThrow([&]() { EXPECT_EQ(m_db->Txns().PullMergeApply(*updatedRevision), ChangesetStatus::Success); }, "Detected 1 foreign key conflicts in ChangeSet. Aborting merge.");

    // Class should be deleted
    checkTestClassExists(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, UpdatingClassAfterDataEntry)
    {
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"UpdatingClassAfterDataEntry.bim");
    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Initialized db"));

    auto context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(m_db->GetSchemaLocater());

    BeFileName searchDirs[2];
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDirs[0]);
    searchDirs[0].AppendToPath(L"ECSchemas");
    searchDirs[1] = searchDirs[0];

    context->AddFirstSchemaPaths({ searchDirs[0].AppendToPath(L"Dgn"), searchDirs[1].AppendToPath(L"Standard") });

    // Set up a base dynamic schema with a class
    const auto schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="1.0.0" alias="CoreCA" />

            <ECCustomAttributes>
                <DynamicSchema xmlns = 'CoreCustomAttributes.1.0.0' />
            </ECCustomAttributes>

            <ECEntityClass typeName="TestClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="TestProperty1" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaPtr initialSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(initialSchema, schemaXml, *context));
    m_db->ImportSchemas({ initialSchema.get() }, true);
    m_db->SaveChanges("Created Test Schema");

    // Create a revision and a backup point
    const auto initialRevision = CreateRevision("-initialize");
    ASSERT_TRUE(initialRevision.IsValid());

    DgnElementId testElementId;
    ChangesetPropsCPtr revision;
    {
        // Insert element
        PhysicalModelPtr model = m_db->Models().Get<PhysicalModel>(m_defaultModelId);
        ASSERT_TRUE(model.IsValid());
        GeometryBuilderPtr builder = GeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));
        builder->Append(*ICurvePrimitive::CreateArc(DEllipse3d::From(1, 2, 3, 0, 0, 2, 0, 3, 0, 0.0, Angle::TwoPi())));

        GenericPhysicalObjectPtr testElement = GenericPhysicalObject::Create(*model, m_defaultCategoryId);
        ASSERT_EQ(SUCCESS, builder->Finish(*testElement));

        testElement->SetPropertyValue("TestProperty1", ECValue("InitialValue"));
        DgnDbStatus statusInsert;
        testElement->Insert(&statusInsert);
        ASSERT_EQ(DgnDbStatus::Success, statusInsert);

        testElementId = testElement->GetElementId();
        ASSERT_TRUE(testElementId.IsValid());
        ASSERT_TRUE(m_db->Elements().GetElement(testElementId).IsValid());
        m_db->SaveChanges("Inserted Element");

        revision = CreateRevision("-insertElement");
        ASSERT_TRUE(revision.IsValid());
    }

    BackupTestFile();

    // Create a changeset to delete the class
    // Perform a major schema update that adds a class property
    const auto updatedSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="1.0.0" alias="CoreCA" />

            <ECCustomAttributes>
                <DynamicSchema xmlns = 'CoreCustomAttributes.1.0.0' />
            </ECCustomAttributes>
            <ECEntityClass typeName="TestClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="TestProperty1" typeName="string" />
                <ECProperty propertyName="TestProperty2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaPtr updatedSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(updatedSchema, updatedSchemaXml, *context));
    m_db->ImportSchemas({ updatedSchema.get() }, true);
    m_db->SaveChanges("Updated Test Schema");

    const auto updatedRevision = CreateRevision("-schemaUpdate");
    ASSERT_TRUE(updatedRevision.IsValid());

    // Now that we have a changeset with a major schema update that deletes the class, restore the imodel to the backup state
    RestoreTestFile();

    // Apply the changeset with the major schema update that deletes the class
    EXPECT_EQ(m_db->Txns().PullMergeApply(*updatedRevision), ChangesetStatus::Success);

    CloseDgnDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, Revert_DeleteClassConstraintViolationInCacheTable)
    {
    auto checkTestClassExists = [&](const bool shouldExist, Utf8StringCR className)
        {
        // Check class existence in schema and cache table in one place
        ASSERT_EQ(shouldExist, nullptr != m_db->Schemas().GetClass("TestSchema", className));

        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db,
            "SELECT 1 FROM ec_cache_ClassHierarchy ch "
            "JOIN ec_Class c ON ch.classId = c.Id WHERE c.Name = ?"));
        stmt.BindText(1, className.c_str(), Statement::MakeCopy::No);
        ASSERT_EQ(shouldExist, stmt.Step() == BE_SQLITE_ROW);
        stmt.Finalize();
        };

    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"DeleteClassConstraintViolationInCacheTable.bim");
    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Initialized db"));

    auto context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(m_db->GetSchemaLocater());

    BeFileName searchDirs[2];
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDirs[0]);
    searchDirs[0].AppendToPath(L"ECSchemas");
    searchDirs[1] = searchDirs[0];

    context->AddFirstSchemaPaths({ searchDirs[0].AppendToPath(L"Dgn"), searchDirs[1].AppendToPath(L"Standard") });

    // Set up a base dynamic schema with a class
    const auto schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="1.0.0" alias="CoreCA" />

            <ECCustomAttributes>
                <DynamicSchema xmlns = 'CoreCustomAttributes.1.0.0' />
            </ECCustomAttributes>

            <ECEntityClass typeName="TestClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaPtr initialSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(initialSchema, schemaXml, *context));
    m_db->ImportSchemas({ initialSchema.get() }, true);
    m_db->SaveChanges("Created Test Schema");

    // Create a revision and a backup point
    const auto initialRevision = CreateRevision("-initialize");
    ASSERT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    // Check if class exists
    checkTestClassExists(true, "TestClass");

    // Create a changeset to delete the class
    // Perform a major schema update that deletes the class
    const auto updatedSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="1.0.0" alias="CoreCA" />

            <ECCustomAttributes>
                <DynamicSchema xmlns = 'CoreCustomAttributes.1.0.0' />
            </ECCustomAttributes>

            <ECEntityClass typeName="TestClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>

            <ECEntityClass typeName="AnotherTestClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaPtr updatedSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(updatedSchema, updatedSchemaXml, *context));
    m_db->ImportSchemas({ updatedSchema.get() }, true);
    m_db->SaveChanges("Updated Test Schema");

    const auto updatedRevision = CreateRevision("-schemaClassAdd");
    ASSERT_TRUE(updatedRevision.IsValid());

    // Classes should be inserted
    checkTestClassExists(true, "TestClass");
    checkTestClassExists(true, "AnotherTestClass");

    m_db->Txns().RevertTimelineChanges({ updatedRevision }, false);

    // "AnotherTestClass" should be deleted
    checkTestClassExists(true, "TestClass");
    checkTestClassExists(false, "AnotherTestClass");
    }

TEST_F(RevisionTestFixture, CheckHealthStatsWithSchemaChanges) {
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"CheckHealthStatsWithSchemaChanges.bim");
    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Initialized db"));
    ASSERT_TRUE(CreateRevision("-initialize").IsValid());
    BackupTestFile();

    auto context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(m_db->GetSchemaLocater());
    BeFileName searchDirs[2];
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDirs[0]);
    searchDirs[0].AppendToPath(L"ECSchemas");
    searchDirs[1] = searchDirs[0];
    context->AddFirstSchemaPaths({ searchDirs[0].AppendToPath(L"Dgn"), searchDirs[1].AppendToPath(L"Standard") });

    auto importSchemaAndCreateRevision = [&](const std::pair<Utf8String, Utf8String>& importSchemaInfo, ECN::ECSchemaReadContext& ctx) -> ChangesetPropsPtr {
        ECSchemaPtr schema;
        if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(schema, importSchemaInfo.first.c_str(), ctx)) {
            ADD_FAILURE() << "Failed to read schema from XML: " << importSchemaInfo.first.c_str();
            return nullptr;
        }
        if (!schema.IsValid()) {
            ADD_FAILURE() << "Schema is not valid: " << importSchemaInfo.first.c_str();
            return nullptr;
        }
        m_db->ImportSchemas({ schema.get() }, true);
        m_db->SaveChanges(importSchemaInfo.second.c_str());
        const auto revision = CreateRevision(importSchemaInfo.second.c_str());
        if (!revision.IsValid()) {
            ADD_FAILURE() << "Failed to create revision: " << importSchemaInfo.second.c_str();
            return nullptr;
        }
        return revision;
    };

    std::vector<std::pair<Utf8String, Utf8String>> importSchemaInfo = { std::make_pair(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>

            <ECEntityClass typeName="TestClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="TestProperty" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", "-importSchema"),

        std::make_pair(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="1.0.0" alias="CoreCA" />

            <ECCustomAttributes>
                <DynamicSchema xmlns = 'CoreCustomAttributes.1.0.0' />
            </ECCustomAttributes>

            <ECEntityClass typeName="TestClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="TestProperty" typeName="int" />
                <ECProperty propertyName="AnotherTestProperty" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml", "-firstUpdate"),

        std::make_pair(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="1.0.0" alias="CoreCA" />
            <ECCustomAttributes>
                <DynamicSchema xmlns = 'CoreCustomAttributes.1.0.0' />
            </ECCustomAttributes>

            <ECEntityClass typeName="TestClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="TestProperty" typeName="int" />
            </ECEntityClass>

            <ECEnumeration typeName="EnumVal" backingTypeName="string" isStrict="true" description="Defines the layers in the BIS schema hierarchy.">
                <ECEnumerator name="First" displayLabel="First" value="First" description="" />
                <ECEnumerator name="Second" displayLabel="Second" value="Second" description="" />
            </ECEnumeration>
        </ECSchema>)xml", "-secondUpdate"),

        std::make_pair(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="2.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="1.0.0" alias="CoreCA" />
            <ECCustomAttributes>
                <DynamicSchema xmlns = 'CoreCustomAttributes.1.0.0' />
            </ECCustomAttributes>

            <ECEnumeration typeName="EnumVal" backingTypeName="string" isStrict="true" description="Defines the layers in the BIS schema hierarchy.">
                <ECEnumerator name="First" displayLabel="First" value="First" description="" />
                <ECEnumerator name="Second" displayLabel="Second" value="Second" description="" />
            </ECEnumeration>
        </ECSchema>)xml", "-majorUpdate") };

    // Import initial schema (not tracked in revisions vector)
    importSchemaAndCreateRevision(importSchemaInfo[0], *context);

    BackupTestFile();
    std::vector<ChangesetPropsPtr> revisions;
    for (int i = 1; i < 4; ++i)
        revisions.push_back(importSchemaAndCreateRevision(importSchemaInfo[i], *context));

    RestoreTestFile();
    m_db->Txns().EnableChangesetHealthStatsTracking();
    for (const auto& revision : revisions)
        MergeSchemaRevision(*revision);

    struct TestCase {
        unsigned int insertedRows;
        unsigned int updatedRows;
        unsigned int deletedRows;
        unsigned int scanCount;
        unsigned int sqlStatementCount;
    };

    std::unordered_map<Utf8String, TestCase> revisionsData = {
        { revisions[0]->GetChangesetId(), { 5, 2, 0, 0, 7 } },
        { revisions[1]->GetChangesetId(), { 1, 1, 3, 0, 5 } },
        { revisions[2]->GetChangesetId(), { 0, 1097, 51, 0, 11 } },
    };

    auto changesets = m_db->Txns().GetAllChangesetHealthStatistics()["changesets"];
    ASSERT_TRUE(changesets.isArray());
    ASSERT_EQ(revisions.size(), changesets.size());

    // Validate health stats for each revision
    changesets.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst changeset) {
        Utf8String id = changeset["changeset_id"].asString();
        auto it = revisionsData.find(id);
        if (it == revisionsData.end()) {
            EXPECT_FALSE(true) << "Changeset " << id << " not found in expected data.";
            return false; // Continue to next iteration
        }
        EXPECT_GT(changeset["uncompressed_size_bytes"].asUInt(), 1U) << "Uncompressed size mismatch for changeset: " << id;
        const auto& expected = it->second;

        if (expected.insertedRows == 0)
            EXPECT_EQ(changeset["inserted_rows"].asUInt(), expected.insertedRows) << "Inserted rows mismatch for changeset: " << id;
        else
            EXPECT_GE(changeset["inserted_rows"].asUInt(), expected.insertedRows) << "Inserted rows mismatch for changeset: " << id;

        EXPECT_GE(changeset["updated_rows"].asUInt(), expected.updatedRows) << "Updated rows mismatch for changeset: " << id;

        if (expected.deletedRows == 0)
            EXPECT_EQ(changeset["deleted_rows"].asUInt(), expected.deletedRows) << "Deleted rows mismatch for changeset: " << id;
        else
            EXPECT_GE(changeset["deleted_rows"].asUInt(), expected.deletedRows) << "Deleted rows mismatch for changeset: " << id;

        EXPECT_EQ(changeset["scan_count"].asUInt(), expected.scanCount) << "Scan count mismatch for changeset: " << id;
        EXPECT_EQ(changeset["health_stats"].size(), expected.sqlStatementCount) << "SQL statement count mismatch for changeset: " << id;

        return false;
    });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, CheckHealthStatsWithElementCRUD) {
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"CheckHealthStatsWithElementCRUD.bim");
    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Initialized db"));

    auto context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(m_db->GetSchemaLocater());

    BeFileName searchDirs[2];
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDirs[0]);
    searchDirs[0].AppendToPath(L"ECSchemas");
    searchDirs[1] = searchDirs[0];

    context->AddFirstSchemaPaths({ searchDirs[0].AppendToPath(L"Dgn"), searchDirs[1].AppendToPath(L"Standard") });

    // Set up a base schema with a class
    const auto baseSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECEntityClass typeName="TestClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="TestProperty" typeName="string" />
                <ECProperty propertyName="AnotherTestProperty" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaPtr initialSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(initialSchema, baseSchemaXml, *context));
    m_db->ImportSchemas({ initialSchema.get() }, true);
    m_db->SaveChanges("Created Test Schema");

    ASSERT_TRUE(CreateRevision("-initialRevision").IsValid());

    BackupTestFile();

    ChangesetPropsPtr revision;
    DgnElementId testElementId;
    // Insert element
    {
        PhysicalModelPtr model = m_db->Models().Get<PhysicalModel>(m_defaultModelId);
        ASSERT_TRUE(model.IsValid());
        GeometryBuilderPtr builder = GeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));
        builder->Append(*ICurvePrimitive::CreateArc(DEllipse3d::From(1, 2, 3, 0, 0, 2, 0, 3, 0, 0.0, Angle::TwoPi())));

        GenericPhysicalObjectPtr testElement = GenericPhysicalObject::Create(*model, m_defaultCategoryId);
        ASSERT_EQ(SUCCESS, builder->Finish(*testElement));

        DgnDbStatus statusInsert;
        testElement->Insert(&statusInsert);
        ASSERT_EQ(DgnDbStatus::Success, statusInsert);

        testElementId = testElement->GetElementId();
        ASSERT_TRUE(testElementId.IsValid());
        ASSERT_TRUE(m_db->Elements().GetElement(testElementId).IsValid());
        m_db->SaveChanges("Inserted Element");

        revision = CreateRevision("-insertElement");
        ASSERT_TRUE(revision.IsValid());
    }

    RestoreTestFile();
    m_db->Txns().EnableChangesetHealthStatsTracking();
    EXPECT_TRUE(m_db->Elements().GetElement(testElementId).IsNull());
    MergeSchemaRevision(*revision);

    const auto insertChangeset = m_db->Txns().GetAllChangesetHealthStatistics()["changesets"];
    EXPECT_EQ(insertChangeset.size(), 1);
    insertChangeset.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst changeset) {
        EXPECT_STREQ(changeset["changeset_id"].asString().c_str(), revision->GetChangesetId().c_str());
        EXPECT_GT(changeset["uncompressed_size_bytes"].asUInt(), 1U);
        EXPECT_EQ(changeset["inserted_rows"].asUInt(), 4);
        EXPECT_EQ(changeset["updated_rows"].asUInt(), 2);
        EXPECT_EQ(changeset["deleted_rows"].asUInt(), 0);
        EXPECT_EQ(changeset["scan_count"].asUInt(), 0);
        EXPECT_EQ(changeset["health_stats"].size(), 6);
        return false;
    });

    BackupTestFile();

    // Update element
    {
        auto testElement = m_db->Elements().GetForEdit<PhysicalElement>(testElementId);
        ASSERT_TRUE(testElement.IsValid());

        testElement->SetPropertyValue("TestProperty", ECValue("UpdatedValue"));
        testElement->SetPropertyValue("AnotherTestProperty", ECValue(20));
        testElement->Update();
        m_db->SaveChanges("Updated Element");

        revision = CreateRevision("-updateElement");
        ASSERT_TRUE(revision.IsValid());
    }

    RestoreTestFile();
    m_db->Txns().EnableChangesetHealthStatsTracking();
    EXPECT_TRUE(m_db->Elements().GetElement(testElementId).IsValid());
    MergeSchemaRevision(*revision);
    
    const auto updateChangeset = m_db->Txns().GetAllChangesetHealthStatistics()["changesets"];
    EXPECT_EQ(updateChangeset.size(), 1);
    updateChangeset.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst changeset) {
        EXPECT_STREQ(changeset["changeset_id"].asString().c_str(), revision->GetChangesetId().c_str());
        EXPECT_GT(changeset["uncompressed_size_bytes"].asUInt(), 1U);
        EXPECT_EQ(changeset["inserted_rows"].asUInt(), 0);
        EXPECT_EQ(changeset["updated_rows"].asUInt(), 4);
        EXPECT_EQ(changeset["deleted_rows"].asUInt(), 0);
        EXPECT_EQ(changeset["scan_count"].asUInt(), 0);
        EXPECT_EQ(changeset["health_stats"].size(), 4);
        return false;
    });

    BackupTestFile();

    // Delete element
    {
        auto testElement = m_db->Elements().GetForEdit<PhysicalElement>(testElementId);
        ASSERT_TRUE(testElement.IsValid());
        testElement->Delete();
        m_db->SaveChanges("Deleted Element");

        revision = CreateRevision("-deleteElement");
        ASSERT_TRUE(revision.IsValid());
    }

    RestoreTestFile();
    m_db->Txns().EnableChangesetHealthStatsTracking();
    EXPECT_TRUE(m_db->Elements().GetElement(testElementId).IsValid());
    MergeSchemaRevision(*revision);
    
    const auto deleteChangeset = m_db->Txns().GetAllChangesetHealthStatistics()["changesets"];
    EXPECT_EQ(deleteChangeset.size(), 1);
    deleteChangeset.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst changeset) {
        EXPECT_STREQ(changeset["changeset_id"].asString().c_str(), revision->GetChangesetId().c_str());
        EXPECT_GT(changeset["uncompressed_size_bytes"].asUInt(), 1U);
        EXPECT_EQ(changeset["inserted_rows"].asUInt(), 1);
        EXPECT_EQ(changeset["updated_rows"].asUInt(), 2);
        EXPECT_EQ(changeset["deleted_rows"].asUInt(), 3);
        EXPECT_EQ(changeset["scan_count"].asUInt(), 0);
        EXPECT_EQ(changeset["health_stats"].size(), 6);
        return false;
    });
}
