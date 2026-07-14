/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/ColorUtil.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_DPTEST

static BeSQLite::BeBriefcaseId TEST_BRIEFCASE_ID = BeSQLite::BeBriefcaseId(1000);

//=======================================================================================
// @bsiclass
//=======================================================================================
struct StepTimer
{
  public:
    WCharCP m_msg;
    uint64_t m_startTime;

    static uint64_t GetCurrentTime() { return BeTimeUtilities::QueryMillisecondsCounter(); }

    StepTimer(WCharCP msg)
    {
        m_msg = msg;
        m_startTime = GetCurrentTime();
    }
    ~StepTimer() { wprintf(L"%ls, %lf Seconds\n", m_msg, (GetCurrentTime() - m_startTime) / 1000.); }
};
//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnDbTest : public DgnDbTestFixture
{
};

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, CheckStandardProperties)
{
    SetupSeedProject();

    DgnDbP project = m_db.get();
    ASSERT_TRUE(project != nullptr);
    Utf8String val;

    // Check that std properties are in the be_Props table. We can only check the value of a few using this API.
    ASSERT_EQ(BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("DbGuid", "be_Db")));
    ASSERT_EQ(BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("BeSQLiteBuild", "be_Db")));
    ASSERT_EQ(BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("CreationDate", "be_Db")));
    ASSERT_EQ(BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("SchemaVersion", "be_Db")));
    ASSERT_EQ(BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("SchemaVersion", "ec_Db")));
    ASSERT_EQ(BE_SQLITE_ROW, project->QueryProperty(val, DgnProjectProperty::Units()));
    ASSERT_EQ(BE_SQLITE_ROW, project->QueryProperty(val, DgnProjectProperty::LastEditor()));
    ASSERT_EQ(BE_SQLITE_ROW, project->QueryProperty(val, DgnProjectProperty::ProfileVersion()));

    // Use the model API to access model properties and check their values
    PhysicalModelPtr defaultModel = GetDefaultPhysicalModel();

    // Use ModelInfo as an alt. way to get at some of the same property data
    GeometricModel::Formatter const &displayInfo = defaultModel->GetFormatter();
    ASSERT_TRUE(displayInfo.GetMasterUnits().GetBase() == UnitBase::Meter);
    ASSERT_TRUE(displayInfo.GetSubUnits().GetBase() == UnitBase::Meter);
}

/*---------------------------------------------------------------------------------**//**
* Schema Version can be accessed and it is correct
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, ProjectProfileVersions)
{
    SetupSeedProject();
    DgnDbP project = m_db.get();
    ASSERT_TRUE(project != nullptr);

    // Get Schema version details
    DgnDbProfileVersion profileVer = project->GetProfileVersion();
    ASSERT_EQ(DGNDB_CURRENT_VERSION_Major, profileVer.GetMajor()) << "The Schema Major Version is: " << profileVer.GetMajor();
    ASSERT_EQ(DGNDB_CURRENT_VERSION_Minor, profileVer.GetMinor()) << "The Schema Minor Version is: " << profileVer.GetMinor();
    ASSERT_EQ(DGNDB_CURRENT_VERSION_Sub1, profileVer.GetSub1()) << "The Schema Sub1 Version is: " << profileVer.GetSub1();
    ASSERT_EQ(DGNDB_CURRENT_VERSION_Sub2, profileVer.GetSub2()) << "The Schema Sub2 Version is: " << profileVer.GetSub2();
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct CategoryTestData
{
    DgnDbR m_db;
    DgnCategoryId m_category1;
    DgnCategoryId m_category2;
    CategoryTestData(DgnDbR db, DgnCategoryId category1, DgnCategoryId category2) : m_db(db), m_category1(category1), m_category2(category2) {}
};

static DgnSubCategoryId facetId1, facetId2;

/*---------------------------------------------------------------------------------**/ /**
* Creating a project with Duplicate name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, ProjectWithDuplicateName)
{
    CreateDgnDbParams params(TEST_NAME);
    DbResult status, status2;
    DgnDbPtr project, project2;

    //Deleting the project file if it exists already
    BeFileName::BeDeleteFile(DgnDbTestDgnManager::GetOutputFilePath(L"dup.ibim"));

    //Create and Verify that project was created
    project = DgnDb::CreateIModel(&status, DgnDbTestDgnManager::GetOutputFilePath(L"dup.ibim"), params);
    ASSERT_TRUE(project != nullptr);
    ASSERT_EQ(BE_SQLITE_OK, status) << "Status returned is:" << status;

    // Close the original project (otherwise, we'll get a sharing violation, rather than a dup name error).
    project = nullptr;

    // Don't allow existing file to be replaced
    params.SetOverwriteExisting(false);

    //Create another project with same name. It should fail
    project2 = DgnDb::CreateIModel(&status2, DgnDbTestDgnManager::GetOutputFilePath(L"dup.ibim"), params);
    EXPECT_FALSE(project2.IsValid()) << "Project with Duplicate name should not be created";
    EXPECT_EQ(BE_SQLITE_ERROR_FileExists, status2) << "Status returned for duplicate name is: " << status2;
}

/*---------------------------------------------------------------------------------**/ /**
* Try multiple read/write for project
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, MultipleReadWrite)
{
    BeFileName testFile;
    ASSERT_TRUE(DgnDbStatus::Success == DgnDbTestFixture::GetSeedDbCopy(testFile, L"MultipleReadWrite.bim"));

    DbResult status1;
    DgnDbPtr dgnProj1;
    dgnProj1 = DgnDb::OpenIModelDb(&status1, testFile, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Exclusive));
    EXPECT_EQ(BE_SQLITE_OK, status1) << status1;
    ASSERT_TRUE(dgnProj1 != nullptr);

    DbResult status2;
    DgnDbPtr dgnProj2;
    dgnProj2 = DgnDb::OpenIModelDb(&status2, testFile, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Exclusive));
    EXPECT_NE(BE_SQLITE_OK, status2) << status2;
    ASSERT_TRUE(dgnProj2 == nullptr);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, InvalidFileFormat)
{
    DgnDbPtr dgnProj;
    BeFileName path;
    StatusInt testDataFound = TestDataManager::FindTestData(path, L"ECSqlTest.01.00.ecschema.xml", BeFileName(L"DgnDb\\Schemas"));
    ASSERT_TRUE(SUCCESS == testDataFound);

    DbResult status;
    dgnProj = DgnDb::OpenIModelDb(&status, path, DgnDb::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ(BE_SQLITE_NOTADB, status) << status;
    ASSERT_TRUE(dgnProj == nullptr);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, CreateIModel)
{
    DgnDbPtr dgnProj;
    BeFileName dgndbFileName;
    BeTest::GetHost().GetOutputRoot(dgndbFileName);
    dgndbFileName.AppendToPath(L"MyFile.ibim");

    if (BeFileName::DoesPathExist(dgndbFileName))
        BeFileName::BeDeleteFile(dgndbFileName);

    DbResult status;
    CreateDgnDbParams params(TEST_NAME);
    dgnProj = DgnDb::CreateIModel(&status, BeFileName(dgndbFileName.GetNameUtf8().c_str()), params);
    EXPECT_EQ(BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(dgnProj != nullptr);
}

/*---------------------------------------------------------------------------------**/ /**
* Verify that an in-memory iModel can be created and written to, and that a secondary connection
* (the mechanism used by Concurrent Query and EC Presentation) shares the same in-memory data.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, CreateInMemoryIModel)
{
    // An empty file name requests an in-memory iModel.
    DbResult status;
    CreateDgnDbParams params(TEST_NAME);
    DgnDbPtr dgndb = DgnDb::CreateIModel(&status, BeFileName(), params);
    ASSERT_EQ(BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(dgndb.IsValid());

    // The database has no backing file, and it must be reachable by a reopen name for secondary connections.
    EXPECT_TRUE(dgndb->IsInMemoryDb());
    EXPECT_TRUE(Utf8String::IsNullOrEmpty(dgndb->GetDbFileName()));
    EXPECT_FALSE(Utf8String::IsNullOrEmpty(dgndb->GetDbFileNameForReopen()));

    // Write some data.
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*dgndb, "TestPartition");
    ASSERT_TRUE(model.IsValid());
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*dgndb, "TestCategory");
    ASSERT_TRUE(categoryId.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, dgndb->SaveChanges());

    // Open a secondary (read-only) connection - the same mechanism used by Concurrent Query and EC
    // Presentation. For a private ":memory:" database this would open a separate empty database and either
    // fail (no property table) or see no data. With the shared-cache in-memory database it must see the
    // data written on the primary connection.
    Db secondaryDb;
    ASSERT_EQ(BE_SQLITE_OK, dgndb->OpenSecondaryConnection(secondaryDb, Db::OpenParams(Db::OpenMode::Readonly, DefaultTxn::No)));

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(secondaryDb, "SELECT count(*) FROM ec_Class"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_GT(stmt.GetValueInt(0), 0) << "secondary connection should see the shared in-memory schema data";
    stmt.Finalize();
    secondaryDb.CloseDb();
}

/*---------------------------------------------------------------------------------**/ /**
* Verify that a shared in-memory iModel can be opened by its shared-cache URI while the creating
* connection is still open.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, OpenInMemoryIModelByUri)
{
    DbResult status;
    CreateDgnDbParams params(TEST_NAME);
    DgnDbPtr dgndb = DgnDb::CreateIModel(&status, BeFileName(), params);
    ASSERT_EQ(BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(dgndb.IsValid());

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*dgndb, "TestPartition");
    ASSERT_TRUE(model.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, dgndb->SaveChanges());

    // Build the shared-cache URI from the reopen name and open a second full connection to the same iModel.
    Utf8String uri;
    uri.Sprintf("file:%s?mode=memory&cache=shared", dgndb->GetDbFileNameForReopen());

    DbResult openStatus;
    DgnDbPtr dgndb2 = DgnDb::OpenIModelDb(&openStatus, BeFileName(uri.c_str(), true), DgnDb::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, openStatus) << openStatus;
    ASSERT_TRUE(dgndb2.IsValid());
    EXPECT_TRUE(dgndb2->IsInMemoryDb());

    // The second connection must see the model inserted on the first connection.
    EXPECT_TRUE(dgndb2->Models().GetModel(model->GetModelId()).IsValid());
}

/*---------------------------------------------------------------------------------**/ /**
* Verify that an existing on-disk iModel can be opened as a writable in-memory copy, and that the
* in-memory copy can be written back out to a new file (the load/save round-trip).
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, OpenInMemoryCopyOfIModel)
{
    // Create an on-disk seed iModel with some data.
    BeFileName seedFileName;
    BeTest::GetHost().GetOutputRoot(seedFileName);
    seedFileName.AppendToPath(L"InMemoryCopySeed.bim");
    if (BeFileName::DoesPathExist(seedFileName))
        BeFileName::BeDeleteFile(seedFileName);

    DgnModelId seedModelId;
    {
        DbResult status;
        CreateDgnDbParams params(TEST_NAME);
        DgnDbPtr seed = DgnDb::CreateIModel(&status, seedFileName, params);
        ASSERT_EQ(BE_SQLITE_OK, status) << status;
        ASSERT_TRUE(seed.IsValid());
        PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*seed, "SeedPartition");
        ASSERT_TRUE(model.IsValid());
        seedModelId = model->GetModelId();
        ASSERT_EQ(BE_SQLITE_OK, seed->SaveChanges());
        seed->CloseDb();
    }

    // Open a writable in-memory copy of the on-disk iModel.
    DbResult status;
    DgnDbPtr memDb = DgnDb::OpenInMemoryCopyOfIModel(&status, seedFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ(BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(memDb.IsValid());
    EXPECT_TRUE(memDb->IsInMemoryDb());

    // The copy must contain the data from the seed file...
    EXPECT_TRUE(memDb->Models().GetModel(seedModelId).IsValid()) << "in-memory copy should contain the seed model";

    // ...and it must be writable, without affecting the source file.
    PhysicalModelPtr memModel = DgnDbTestUtils::InsertPhysicalModel(*memDb, "InMemoryPartition");
    ASSERT_TRUE(memModel.IsValid());
    DgnModelId memModelId = memModel->GetModelId();
    ASSERT_EQ(BE_SQLITE_OK, memDb->SaveChanges());

    // Write the in-memory copy (including the new model) out to a new file.
    BeFileName outFileName;
    BeTest::GetHost().GetOutputRoot(outFileName);
    outFileName.AppendToPath(L"InMemoryCopyOut.bim");
    if (BeFileName::DoesPathExist(outFileName))
        BeFileName::BeDeleteFile(outFileName);
    ASSERT_EQ(BE_SQLITE_OK, memDb->VacuumInto(outFileName.GetNameUtf8().c_str()));
    memDb->CloseDb();

    // The source file must be unchanged (no new model), while the saved-out file must contain both models.
    {
        DbResult openStat;
        DgnDbPtr reopenedSeed = DgnDb::OpenIModelDb(&openStat, seedFileName, DgnDb::OpenParams(Db::OpenMode::Readonly));
        ASSERT_EQ(BE_SQLITE_OK, openStat) << openStat;
        EXPECT_TRUE(reopenedSeed->Models().GetModel(seedModelId).IsValid());
        EXPECT_FALSE(reopenedSeed->Models().GetModel(memModelId).IsValid()) << "source file must not be modified by in-memory edits";
    }
    {
        DbResult openStat;
        DgnDbPtr saved = DgnDb::OpenIModelDb(&openStat, outFileName, DgnDb::OpenParams(Db::OpenMode::Readonly));
        ASSERT_EQ(BE_SQLITE_OK, openStat) << openStat;
        EXPECT_TRUE(saved->Models().GetModel(seedModelId).IsValid());
        EXPECT_TRUE(saved->Models().GetModel(memModelId).IsValid()) << "saved-out file must contain in-memory edits";
    }
}


/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, SetBriefcaseAsStandalone)
{
    // TFS#905753 - Setup an untracked DB as a master copy after local changes
    DbResult result = BE_SQLITE_ERROR;
    CreateDgnDbParams params(TEST_NAME);
    auto filename =  DgnDbTestDgnManager::GetOutputFilePath(L"MasterCopy.ibim");
    DgnDbPtr dgndb = DgnDb::CreateIModel(&result, filename, params);
    ASSERT_TRUE(dgndb.IsValid());

    dgndb->ResetBriefcaseId(TEST_BRIEFCASE_ID);
    ASSERT_TRUE(dgndb->Txns().IsTracking());

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*dgndb, "TestPartition");
    ASSERT_TRUE(model.IsValid());
    dgndb->SaveChanges();

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*dgndb, "TestCategory");
    ASSERT_TRUE(categoryId.IsValid());
    dgndb->SaveChanges();
    dgndb->CloseDb();

    DbResult stat;
    dgndb = DgnDb::OpenIModelDb(&stat, filename, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(dgndb->Txns().HasPendingTxns());

    dgndb->SaveProjectGuid(BeGuid(true));

    // Check that we can't turn the Briefcase -> Standalone with pending txns
    bool caught = false;
    try {
        dgndb->ResetBriefcaseId(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
    } catch (...) {
        caught = true;
    }
    ASSERT_TRUE(caught);
    dgndb->SaveProjectGuid(BeGuid(true)); // change project id again so we're sure to have an active txn

    dgndb->Txns().DeleteAllTxns(); // should work after we delete all txns
    ASSERT_FALSE(dgndb->Txns().HasPendingTxns());
    ASSERT_TRUE(BE_SQLITE_OK == dgndb->ResetBriefcaseId(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone())));
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, ImportSchemaWithLocalChanges)
{
    // importing schema into a briefcase with local changes should be possible.
    DbResult result = BE_SQLITE_ERROR;
    CreateDgnDbParams params(TEST_NAME);
    DgnDbPtr dgndb = DgnDb::CreateIModel(&result, DgnDbTestDgnManager::GetOutputFilePath(L"ImportSchemaWithLocalChanges.ibim"), params);
    // Fails on Linux
    ASSERT_TRUE(dgndb.IsValid());

    dgndb->ResetBriefcaseId(TEST_BRIEFCASE_ID);
    ASSERT_TRUE(dgndb->Txns().IsTracking());

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*dgndb, "TestPartition");
    ASSERT_TRUE(model.IsValid());

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*dgndb, "TestCategory");
    ASSERT_TRUE(categoryId.IsValid());

    dgndb->SaveChanges();

    SchemaStatus schemaStatus = DgnPlatformTestDomain::GetDomain().ImportSchema(*dgndb);

    ASSERT_TRUE(schemaStatus == SchemaStatus::Success);

    dgndb->SaveChanges();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, DropSchemas)
    {
    auto result = BE_SQLITE_ERROR;
    CreateDgnDbParams params(TEST_NAME);
    auto dgndb = DgnDb::CreateIModel(&result, DgnDbTestDgnManager::GetOutputFilePath(L"DropSchemas.bim"), params);
    ASSERT_TRUE(dgndb.IsValid());

    auto schemaContext = ECN::ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater(dgndb->GetSchemaLocater());

    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaContext->AddSchemaLocater(dgndb->GetSchemaLocater());
    schemaContext->AddSchemaPath(searchDir.GetName());

    ECSchemaPtr schema = nullptr;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, R"xml(
        <ECSchema schemaName="TestSchema1" alias="ts1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECEntityClass typeName="TestClass1">
                <ECCustomAttributes>
                    <ClassHasHandler xmlns="BisCore.1.0.0" />
                </ECCustomAttributes>
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", *schemaContext));
    ASSERT_TRUE(schema.IsValid());

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, R"xml(
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
            <ECSchemaReference name="TestSchema1" version="1.0.0" alias="ts1"/>
            <ECEntityClass typeName="TestClass2">
                <ECCustomAttributes>
                    <ClassHasHandler xmlns="BisCore.1.0.0" />
                </ECCustomAttributes>
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="Prop2" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", *schemaContext));
    ASSERT_TRUE(schema.IsValid());

    ASSERT_EQ(SchemaStatus::Success, dgndb->ImportSchemas(schemaContext->GetCache().GetSchemas(), true));
    dgndb->SaveChanges();

    ASSERT_TRUE(dgndb->Schemas().ContainsSchema("TestSchema1"));
    ASSERT_TRUE(dgndb->Schemas().ContainsSchema("TestSchema2"));

    EXPECT_TRUE(dgndb->DropSchemas({"TestSchema1", "TestSchema2"}).IsSuccess());
    EXPECT_FALSE(dgndb->Schemas().ContainsSchema("TestSchema1"));
    EXPECT_FALSE(dgndb->Schemas().ContainsSchema("TestSchema2"));
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, CreateWithInvalidName)
{
    DgnDbPtr dgnProj;

    BeFileName dgndbFileName;
    BeTest::GetHost().GetOutputRoot(dgndbFileName);
    dgndbFileName.AppendToPath(L"MyTextDGNDBFILE.txt");
    if (BeFileName::DoesPathExist(dgndbFileName))
        BeFileName::BeDeleteFile(dgndbFileName);

    DbResult status;
    CreateDgnDbParams params(TEST_NAME);
    dgnProj = DgnDb::CreateIModel(&status, BeFileName(dgndbFileName.GetNameUtf8().c_str()), params);
    EXPECT_EQ(BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(dgnProj != nullptr);
    /////////It creates a DgnDbfile with .txt extension having success status needs to figure out is this right behavior
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, FileNotFoundToOpen)
{
    DbResult status;
    CreateDgnDbParams Obj;
    DgnDbPtr dgnProj;

    BeFileName dgndbFileNotExist;
    BeTest::GetHost().GetOutputRoot(dgndbFileNotExist);
    dgndbFileNotExist.AppendToPath(L"MyFileNotExist.ibim");

    dgnProj = DgnDb::OpenIModelDb(&status, BeFileName(dgndbFileNotExist.GetNameUtf8().c_str()), DgnDb::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ(BE_SQLITE_ERROR_FileNotFound, status) << status;
    ASSERT_TRUE(dgnProj == nullptr);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, OpenAlreadyOpen)
{
    BeFileName dgndbFileName;
    ASSERT_TRUE(DgnDbStatus::Success == DgnDbTestFixture::GetSeedDbCopy(dgndbFileName, L"OpenAlreadyOpen.bim"));

    DbResult status;
    DgnDbPtr dgnProj = DgnDb::OpenIModelDb(&status, dgndbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Exclusive));
    EXPECT_EQ(BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(dgnProj != nullptr);

    // once a Db is opened for ReadWrite with exclusive access, it can't be opened, even for read.
    DgnDbPtr dgnProj1 = DgnDb::OpenIModelDb(&status, dgndbFileName, DgnDb::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ(BE_SQLITE_BUSY, status) << status;
    ASSERT_TRUE(dgnProj1 == nullptr);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DgnDbTest, IsPurgeOperationActive)
    {
    CreateDgnDbParams params(TEST_NAME);
    DgnDbPtr db = DgnDb::CreateIModel(nullptr, DgnDbTestDgnManager::GetOutputFilePath(L"IsPurgeOperationActive.bim"), params);
    ASSERT_TRUE(db.IsValid());

    db->BeginPurgeOperation();
    ASSERT_TRUE(db->IsPurgeOperationActive());
    db->EndPurgeOperation();
    ASSERT_FALSE(db->IsPurgeOperationActive());

    {
    DgnDb::PurgeOperation purgeOperation(*db);
    ASSERT_TRUE(db->IsPurgeOperationActive());
    }
    ASSERT_FALSE(db->IsPurgeOperationActive());

    {
    DgnDb::PurgeOperation purgeOperation(*db);
    ASSERT_TRUE(db->IsPurgeOperationActive());

        { // test nested purge operations
        DgnDb::PurgeOperation purgeOperation(*db);
        ASSERT_TRUE(db->IsPurgeOperationActive());
        }

    ASSERT_TRUE(db->IsPurgeOperationActive());
    }
    ASSERT_FALSE(db->IsPurgeOperationActive());
    }
    
TEST_F(DgnDbTest, CreateImodel_ShouldLogLessWarnings)
    {
    // Log to console
    // NativeLogging::Logging::SetLogger(&NativeLogging::ConsoleLogger::GetLogger());
    // NativeLogging::ConsoleLogger::GetLogger().SetSeverity("SQLite", BentleyApi::NativeLogging::LOG_TRACE);
    
    TestLogger testLogger;
    LogCatcher logCatcher(testLogger);

    CreateDgnDbParams params("EmptyModelTest");
    DgnDbPtr db = DgnDb::CreateIModel(nullptr, DgnDbTestDgnManager::GetOutputFilePath(L"EmptyModelTest.bim"), params);
    ASSERT_TRUE(db.IsValid());

    int warningCount = 0;
    for (const auto& message : testLogger.m_messages) {
        if (message.first == NativeLogging::SEVERITY::LOG_WARNING) {
            ++warningCount;
        }
    }
    ASSERT_LT(warningCount, 50);
    }



//----------------------------------------------------------------------------------------
// @bsiclass
//----------------------------------------------------------------------------------------
struct DgnProjectPackageTest : public DgnDbTestFixture
{
  public:
    //ScopedDgnHost m_autoDgnHost;

    /*---------------------------------------------------------------------------------**/ /**
        * @bsiclass
        +---------------+---------------+---------------+---------------+---------------+------*/
    struct PropertiesInTable
    {
        Utf8String version;
        Utf8String name;
        Utf8String description;
        Utf8String client;
        Utf8String lastEditor;
        Utf8String creationDate;
    };
    /*---------------------------------------------------------------------------------**/ /**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
    void getPropertiesInTable(DgnDbPtr &project, PropertiesInTable &properties)
    {
        project->QueryProperty(properties.version, DgnProjectProperty::ProfileVersion());
        project->QueryProperty(properties.name, DgnProjectProperty::Name());
        project->QueryProperty(properties.description, DgnProjectProperty::Description());
        project->QueryProperty(properties.client, DgnProjectProperty::Client());
        project->QueryProperty(properties.lastEditor, DgnProjectProperty::LastEditor());
        project->QueryProperty(properties.creationDate, Properties::CreationDate());
    }
    /*---------------------------------------------------------------------------------**/ /**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
    void getPackageProperties(BeSQLite::Db &db, PropertiesInTable &properties, uint64_t embeddedFileId)
    {
        db.QueryProperty(properties.version, DgnEmbeddedProjectProperty::ProfileVersion(), embeddedFileId);
        db.QueryProperty(properties.name, DgnEmbeddedProjectProperty::Name(), embeddedFileId);
        db.QueryProperty(properties.description, DgnEmbeddedProjectProperty::Description(), embeddedFileId);
        db.QueryProperty(properties.client, DgnEmbeddedProjectProperty::Client(), embeddedFileId);
        db.QueryProperty(properties.lastEditor, DgnEmbeddedProjectProperty::LastEditor(), embeddedFileId);
        db.QueryProperty(properties.creationDate, DgnEmbeddedProjectProperty::CreationDate(), embeddedFileId);
    }
    /*---------------------------------------------------------------------------------**/ /**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
    void compareProperties(PropertiesInTable &prop, PropertiesInTable &propV)
    {
        EXPECT_TRUE(prop.version.CompareTo(propV.version) == 0) << "Version does not match";
        EXPECT_TRUE(prop.name.CompareTo(propV.name) == 0) << "Name does not match";
        EXPECT_TRUE(prop.description.CompareTo(propV.description) == 0) << "Description does not match";
        EXPECT_TRUE(prop.client.CompareTo(propV.client) == 0) << "Client does not match";
        EXPECT_TRUE(prop.lastEditor.CompareTo(propV.lastEditor) == 0) << "Last editor does not match";
        EXPECT_TRUE(prop.creationDate.CompareTo(propV.creationDate) == 0) << "Creation date does not match";
    }
    /*---------------------------------------------------------------------------------**/ /**
        * @bsiclass
        +---------------+---------------+---------------+---------------+---------------+------*/
    struct ProjectProperties
    {
        uint32_t modelCount;
        size_t spatialCategoryCount;
        size_t viewCount;
        size_t styleCount;
    };
    /*---------------------------------------------------------------------------------**/ /**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
    void getProjectProperties(DgnDbPtr &project, ProjectProperties &properties)
    {
        properties.modelCount = 0;
        for (ModelIteratorEntryCR entry : project->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)))
        {
            UNUSED_VARIABLE(entry);
            properties.modelCount++;
        }
        properties.viewCount = ViewDefinition::QueryCount(*project);
        properties.spatialCategoryCount = SpatialCategory::MakeIterator(*project).BuildIdSet<DgnCategoryId>().size();
#ifdef STYLE_REWRITE_06
        properties.styleCount = project->LineStyles().MakeIterator().QueryCount();
#else
        properties.styleCount = 0;
#endif
    }
    /*---------------------------------------------------------------------------------**/ /**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
    void compareProjectProperties(ProjectProperties &projProp, ProjectProperties &projPropV)
    {
        EXPECT_EQ(projProp.modelCount, projPropV.modelCount) << "Model count does not match";
        EXPECT_EQ(projProp.spatialCategoryCount, projPropV.spatialCategoryCount) << "SpatialCategory count does not match";
        EXPECT_EQ(projProp.viewCount, projPropV.viewCount) << "View count does not match";
        EXPECT_EQ(projProp.styleCount, projPropV.styleCount) << "Style count does not match";
    }
};

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ImportSchema(ECSchemaR ecSchema, DgnDbR project)
{
    ECSchemaCachePtr schemaList = ECSchemaCache::Create();
    schemaList->AddSchema(ecSchema);
    return (SchemaStatus::Success == project.ImportSchemas(schemaList->GetSchemas())) ? SUCCESS : ERROR;
}

struct ImportTests : DgnDbTestFixture
{
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ImportTests, SimpleSchemaImport)
{
    Utf8CP testSchemaXml = "<ECSchema schemaName=\"TestSchema\" alias=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
                           "  <ECSchemaReference name = 'BisCore' version = '01.00' alias = 'bis' />"
                           "  <ECEntityClass typeName='Element1' >"
                           "    <ECCustomAttributes>"
                           "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
                           "    </ECCustomAttributes>"
                           "    <BaseClass>bis:PhysicalElement</BaseClass>"
                           "    <ECProperty propertyName='Prop1_1' typeName='string' />"
                           "    <ECProperty propertyName='Prop1_2' typeName='long' />"
                           "    <ECProperty propertyName='Prop1_3' typeName='double' />"
                           "  </ECEntityClass>"
                           "  <ECEntityClass typeName='Element2' >"
                           "    <ECCustomAttributes>"
                           "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
                           "    </ECCustomAttributes>"
                           "    <BaseClass>Element1</BaseClass>"
                           "    <ECProperty propertyName='Prop2_1' typeName='string' />"
                           "    <ECProperty propertyName='Prop2_2' typeName='long' />"
                           "    <ECProperty propertyName='Prop2_3' typeName='double' />"
                           "  </ECEntityClass>"
                           "  <ECEntityClass typeName='Element3' >"
                           "    <ECCustomAttributes>"
                           "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
                           "    </ECCustomAttributes>"
                           "    <BaseClass>Element2</BaseClass>"
                           "    <ECProperty propertyName='Prop3_1' typeName='string' />"
                           "    <ECProperty propertyName='Prop3_2' typeName='long' />"
                           "    <ECProperty propertyName='Prop3_3' typeName='double' />"
                           "  </ECEntityClass>"
                           "  <ECEntityClass typeName='Element4' >"
                           "    <ECCustomAttributes>"
                           "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
                           "    </ECCustomAttributes>"
                           "    <BaseClass>Element3</BaseClass>"
                           "    <ECProperty propertyName='Prop4_1' typeName='string' />"
                           "    <ECProperty propertyName='Prop4_2' typeName='long' />"
                           "    <ECProperty propertyName='Prop4_3' typeName='double' />"
                           "  </ECEntityClass>"
                           "  <ECEntityClass typeName='Element4b' >"
                           "    <ECCustomAttributes>"
                           "       <ClassHasHandler xmlns=\"BisCore.01.00\" />"
                           "    </ECCustomAttributes>"
                           "    <BaseClass>Element3</BaseClass>"
                           "    <ECProperty propertyName='Prop4b_1' typeName='string' />"
                           "    <ECProperty propertyName='Prop4b_2' typeName='long' />"
                           "    <ECProperty propertyName='Prop4b_3' typeName='double' />"
                           "    <ECProperty propertyName='Prop4b_4' typeName='point3d' />"
                           "  </ECEntityClass>"
                           "</ECSchema>";

    SetupSeedProject();
    m_db->SaveChanges();
    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater(m_db->GetSchemaLocater());

    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaContext->AddSchemaLocater(m_db->GetSchemaLocater());
    schemaContext->AddSchemaPath(searchDir.GetName());

    ECN::ECSchemaPtr schema = nullptr;
    ASSERT_EQ(ECN::SchemaReadStatus::Success, ECN::ECSchema::ReadFromXmlString(schema, testSchemaXml, *schemaContext));
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(SchemaStatus::Success, m_db->ImportSchemas(schemaContext->GetCache().GetSchemas(), true));
    ASSERT_TRUE(m_db->IsDbOpen());
}

//---------------------------------------------------------------------------------------
// BisCore restricts subclasses of RoleElement. This test confirms AssetElement as a known exception.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ImportTests, AssetSchemaImport)
   {
   Utf8CP schemaXml =
       "<ECSchema schemaName=\"Asset\" alias=\"asset\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
       "  <ECSchemaReference name='BisCore' version='01.00' alias='bis'/>"
       "  <ECEntityClass typeName='AssetElement' modifier='Abstract'>"
       "    <BaseClass>bis:RoleElement</BaseClass>"
       "  </ECEntityClass>"
       "  <ECEntityClass typeName='AssetModel'>"
       "    <BaseClass>bis:RoleModel</BaseClass>"
       "  </ECEntityClass>"
       "</ECSchema>";

   SetupSeedProject();
   ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
   schemaContext->AddSchemaLocater(m_db->GetSchemaLocater());

   ECSchemaPtr schema;
   SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
   ASSERT_EQ(SchemaReadStatus::Success, schemaStatus);
   }
