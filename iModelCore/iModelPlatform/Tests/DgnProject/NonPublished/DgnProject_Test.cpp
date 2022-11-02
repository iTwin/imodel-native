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
    ASSERT_TRUE(project != NULL);
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
    ASSERT_TRUE(project != NULL);

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
    project = DgnDb::CreateDgnDb(&status, DgnDbTestDgnManager::GetOutputFilePath(L"dup.ibim"), params);
    ASSERT_TRUE(project != NULL);
    ASSERT_EQ(BE_SQLITE_OK, status) << "Status returned is:" << status;

    // Close the original project (otherwise, we'll get a sharing violation, rather than a dup name error).
    project = nullptr;

    // Don't allow existing file to be replaced
    params.SetOverwriteExisting(false);

    //Create another project with same name. It should fail
    project2 = DgnDb::CreateDgnDb(&status2, DgnDbTestDgnManager::GetOutputFilePath(L"dup.ibim"), params);
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
    dgnProj1 = DgnDb::OpenDgnDb(&status1, testFile, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Exclusive));
    EXPECT_EQ(BE_SQLITE_OK, status1) << status1;
    ASSERT_TRUE(dgnProj1 != NULL);

    DbResult status2;
    DgnDbPtr dgnProj2;
    dgnProj2 = DgnDb::OpenDgnDb(&status2, testFile, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Exclusive));
    EXPECT_NE(BE_SQLITE_OK, status2) << status2;
    ASSERT_TRUE(dgnProj2 == NULL);
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
    dgnProj = DgnDb::OpenDgnDb(&status, path, DgnDb::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ(BE_SQLITE_NOTADB, status) << status;
    ASSERT_TRUE(dgnProj == NULL);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, CreateDgnDb)
{
    DgnDbPtr dgnProj;
    BeFileName dgndbFileName;
    BeTest::GetHost().GetOutputRoot(dgndbFileName);
    dgndbFileName.AppendToPath(L"MyFile.ibim");

    if (BeFileName::DoesPathExist(dgndbFileName))
        BeFileName::BeDeleteFile(dgndbFileName);

    DbResult status;
    CreateDgnDbParams params(TEST_NAME);
    dgnProj = DgnDb::CreateDgnDb(&status, BeFileName(dgndbFileName.GetNameUtf8().c_str()), params);
    EXPECT_EQ(BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(dgnProj != NULL);
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
    DgnDbPtr dgndb = DgnDb::CreateDgnDb(&result, filename, params);
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
    dgndb = DgnDb::OpenDgnDb(&stat, filename, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
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
    // TFS#906843 was resolved as WAD. This ensures that that's really WAD - i.e.,
    // importing schema into a briefcase with local changes should NOT be possible.
    DbResult result = BE_SQLITE_ERROR;
    CreateDgnDbParams params(TEST_NAME);
    DgnDbPtr dgndb = DgnDb::CreateDgnDb(&result, DgnDbTestDgnManager::GetOutputFilePath(L"ImportSchemaWithLocalChanges.ibim"), params);
    // Fails on Linux
    ASSERT_TRUE(dgndb.IsValid());

    dgndb->ResetBriefcaseId(TEST_BRIEFCASE_ID);
    ASSERT_TRUE(dgndb->Txns().IsTracking());

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*dgndb, "TestPartition");
    ASSERT_TRUE(model.IsValid());

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*dgndb, "TestCategory");
    ASSERT_TRUE(categoryId.IsValid());

    dgndb->SaveChanges();

    BeTest::SetFailOnAssert(false);
    SchemaStatus schemaStatus = DgnPlatformTestDomain::GetDomain().ImportSchema(*dgndb);
    BeTest::SetFailOnAssert(true);

    ASSERT_TRUE(schemaStatus != SchemaStatus::Success);
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
    dgnProj = DgnDb::CreateDgnDb(&status, BeFileName(dgndbFileName.GetNameUtf8().c_str()), params);
    EXPECT_EQ(BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(dgnProj != NULL);
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

    dgnProj = DgnDb::OpenDgnDb(&status, BeFileName(dgndbFileNotExist.GetNameUtf8().c_str()), DgnDb::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ(BE_SQLITE_ERROR_FileNotFound, status) << status;
    ASSERT_TRUE(dgnProj == NULL);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, OpenAlreadyOpen)
{
    BeFileName dgndbFileName;
    ASSERT_TRUE(DgnDbStatus::Success == DgnDbTestFixture::GetSeedDbCopy(dgndbFileName, L"OpenAlreadyOpen.bim"));

    DbResult status;
    DgnDbPtr dgnProj = DgnDb::OpenDgnDb(&status, dgndbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Exclusive));
    EXPECT_EQ(BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(dgnProj != NULL);

    // once a Db is opened for ReadWrite with exclusive access, it can't be opened, even for read.
    DgnDbPtr dgnProj1 = DgnDb::OpenDgnDb(&status, dgndbFileName, DgnDb::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ(BE_SQLITE_BUSY, status) << status;
    ASSERT_TRUE(dgnProj1 == NULL);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DgnDbTest, IsPurgeOperationActive)
    {
    CreateDgnDbParams params(TEST_NAME);
    DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, DgnDbTestDgnManager::GetOutputFilePath(L"IsPurgeOperationActive.bim"), params);
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

    ASSERT_EQ(SchemaStatus::Success, m_db->ImportSchemas(schemaContext->GetCache().GetSchemas()));
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
