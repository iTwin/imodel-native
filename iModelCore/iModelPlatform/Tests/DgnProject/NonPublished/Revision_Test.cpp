/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

    DgnRevisionPtr CreateRevision();
    void DumpRevision(DgnRevisionCR revision, Utf8CP summary = nullptr);

    void BackupTestFile();
    void RestoreTestFile(Db::OpenMode openMode = Db::OpenMode::ReadWrite);

    void ExtractCodesFromRevision(DgnCodeSet& assigned, DgnCodeSet& discarded);
    void ProcessSchemaRevision(DgnRevisionCR revision, RevisionProcessOption revisionProcessOption);
    void MergeSchemaRevision(DgnRevisionCR revision) {
        EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(revision));
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
    DgnRevisionPtr rev = db->Revisions().StartCreateRevision();
    BeAssert(rev.IsValid());
    db->Revisions().FinishCreateRevision(-1);

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
DgnRevisionPtr RevisionTestFixture::CreateRevision()
    {
    DgnRevisionPtr revision = m_db->Revisions().StartCreateRevision();
    if (!revision.IsValid())
        return nullptr;

    RevisionStatus status = m_db->Revisions().FinishCreateRevision(-1);
    if (RevisionStatus::Success != status)
        {
        BeAssert(false);
        return nullptr;
        }

    return revision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RevisionTestFixture::ProcessSchemaRevision(DgnRevisionCR revision, RevisionProcessOption revisionProcessOption)
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, ResetIdSequencesAfterApply)
    {
    // Setup a test file
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"ResetElementIdSequenceAfterApply.bim");
    m_db->SaveChanges("Created Initial Model");

    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());

    DgnElementId idBeforeInserts;
    DbResult result = m_db->GetElementIdSequence().GetNextValue(idBeforeInserts);
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Backup the test file
    BackupTestFile();

    // Make some element inserts, and create the baseline last sequence id (for later comparision)
    InsertFloor(1, 1);
    m_db->SaveChanges("Inserted floor");

    DgnRevisionPtr revision = CreateRevision();
    ASSERT_TRUE(revision.IsValid());

    DgnElementId idAfterInserts;
    result = m_db->GetElementIdSequence().GetNextValue(idAfterInserts);
    ASSERT_TRUE(result == BE_SQLITE_OK);
    ASSERT_GE(idAfterInserts, idBeforeInserts);

    // Restore baseline file, apply the change sets with the same element inserts, and validate the last sequence id
    RestoreTestFile();

    RevisionStatus status = m_db->Revisions().MergeRevision(*revision);
    ASSERT_TRUE(status == RevisionStatus::Success);

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
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    // Create Revision 1 (Schema changes - creating two tables)
    m_db->CreateTable("TestTable1", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    m_db->CreateTable("TestTable2", "Id INTEGER PRIMARY KEY, Column1 INTEGER");

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDdlChanges());
    ASSERT_TRUE(m_db->Txns().HasChanges());

    m_db->SaveChanges("Revision 1");
    DgnRevisionPtr revision1 = CreateRevision();
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
    DgnRevisionPtr revision2 = CreateRevision();
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
    DgnRevisionPtr revision3 = CreateRevision();
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
    DgnRevisionPtr revision4 = CreateRevision();
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
    EXPECT_EQ(RevisionStatus::Success, m_db->Revisions().MergeRevision(*revision2));

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
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(RevisionStatus::ReverseOrReinstateSchemaChanges, m_db->Revisions().ReverseRevision(*revision4));

    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    CloseDgnDb();
    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(*revision4, RevisionProcessOption::Reverse));
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_FALSE(m_db.IsValid()) << "Opening with schema changeset reverse parameters should fail.";
    BeTest::SetFailOnAssert(true);
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
// @bsimethod
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
    DgnRevisionPtr initialRevision = CreateRevision();
    ASSERT_TRUE(initialRevision.IsValid());
    BackupTestFile();

    /* Create revision with schema changes */
    ASSERT_EQ(m_db->AddColumnToTable("TestTable", "Column2", "INTEGER"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->CreateIndex("idx_TestTable_Column1", "TestTable", false, "Column1"), BE_SQLITE_OK);

    ASSERT_FALSE(m_db->Txns().HasDataChanges());
    ASSERT_TRUE(m_db->Txns().HasDdlChanges());

    m_db->SaveChanges("Schema changes");
    DgnRevisionPtr schemaChangesRevision = CreateRevision();
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
    DgnRevisionPtr dataChangesRevision = CreateRevision();
    ASSERT_TRUE(dataChangesRevision.IsValid());
    DumpRevision(*dataChangesRevision, "Revision with data changes:");

    /* Create new revision with more data changes on top of the previous schema changes */
    ASSERT_EQ(m_db->ExecuteSql("UPDATE TestTable SET Column2=1 WHERE Id=1"), BE_SQLITE_OK);
    ASSERT_EQ(m_db->ExecuteSql("INSERT INTO TestTable(Id,Column1,Column2) VALUES(2,2,2)"), BE_SQLITE_OK);

    ASSERT_TRUE(m_db->Txns().HasDataChanges());
    ASSERT_FALSE(m_db->Txns().HasDdlChanges());

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
// @bsimethod
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
// @bsimethod
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
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    ASSERT_STREQ(m_db->Revisions().GetParentRevisionId().c_str(), revisionPtrs[6]->GetChangesetId().c_str());

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
    BeTest::SetFailOnAssert(false);
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    BeTest::SetFailOnAssert(true);
    ASSERT_FALSE(m_db.IsValid()) << "Could perform an invalid merge";

    openParams.GetSchemaUpgradeOptionsR().Reset();
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    /*
    * Test 3: Reinstate and merge
    */
    // Reinstate 5-6
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 5, 6);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Merge);
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    ASSERT_STREQ(m_db->Revisions().GetParentRevisionId().c_str(), revisionPtrs[6]->GetChangesetId().c_str());

    // Merge Rev 7-9
    m_db->CloseDb();
    processRevisions = filterRevisions(revisionPtrs, 7, 9);
    openParams.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(processRevisions, RevisionProcessOption::Merge);
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    ASSERT_STREQ(m_db->Revisions().GetParentRevisionId().c_str(), revisionPtrs[9]->GetChangesetId().c_str());
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionTestFixture, IterateOverRedundantSchemaChange)
    {
    constexpr size_t revCount = 5;
    std::array<DgnRevisionPtr, revCount> revisionPtrs;
    auto& initialRevision = revisionPtrs[0];

    // Setup baseline
    SetupDgnDb(RevisionTestFixture::s_seedFileInfo.fileName, L"RevisionChangesStreamLargeRead.bim");
    m_db->SaveChanges("Created Initial Model");
    initialRevision = CreateRevision();
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

    revisionPtrs[1] = CreateRevision();
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
    revisionPtrs[2] = CreateRevision();
    ASSERT_TRUE(revisionPtrs[2].IsValid());

    // Revision 3 (Delete elements from schema)
    EXPECT_EQ(DgnDbStatus::Success, persistedElement->Delete());
    EXPECT_EQ(DgnDbStatus::Success, testModel->Delete());
    EXPECT_EQ(DgnDbStatus::Success, testPartition->Delete());

    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Revision 3"));
    revisionPtrs[3] = CreateRevision();
    ASSERT_TRUE(revisionPtrs[3].IsValid());

    // Revision 4 (Delete schema)
    ASSERT_TRUE(m_db->DropSchema(schemaName).IsSuccess());

    EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Revision 4"));
    revisionPtrs[4] = CreateRevision();
    ASSERT_TRUE(revisionPtrs[4].IsValid());

    // now load the revisions into change iterators

    bvector<BeFileName> revisionFileNames;
    revisionFileNames.reserve(revisionPtrs.size() - 1);
    // ignore the initial revision since it will have a bunch of inserted stuff for setup purposes that we're not trying to iterate over
    std::transform(revisionPtrs.begin() + 1, revisionPtrs.end(), std::back_inserter(revisionFileNames), [](const auto& r){ return r->GetRevisionChangesFile(); });

    // ignore the initial revision since it will have a bunch of inserted stuff for setup purposes that we're not trying to iterate over
    //RevisionChangesFileReader reader1(revisionPtrs[0]->GetRevisionChangesFile(), *m_db);
    RevisionChangesFileReader reader2(revisionPtrs[1]->GetRevisionChangesFile(), *m_db);
    RevisionChangesFileReader reader3(revisionPtrs[2]->GetRevisionChangesFile(), *m_db);
    RevisionChangesFileReader reader4(revisionPtrs[3]->GetRevisionChangesFile(), *m_db);
    RevisionChangesFileReader reader5(revisionPtrs[4]->GetRevisionChangesFile(), *m_db);
    // this pointer array compensates for a lack of copy or move constructors which make this type difficult to put in STL containers
    std::array<RevisionChangesFileReader*, revisionPtrs.size() - 1> revisionReaders { &reader2, &reader3, &reader4, &reader5  };

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
