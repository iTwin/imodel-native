/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnIModel.h>
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

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/11
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
// @bsiclass                                        Umar.Hayat              07/16
//=======================================================================================
struct DgnDbTest : public DgnDbTestFixture
{
};

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson      03/2012
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
* @bsimethod                                    Shaun.Sewall                    10/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, ConnectedContextId)
    {
    SetupSeedProject();

    BeGuid fakeContextId1(true);
    BeGuid fakeContextId2(true);
    Utf8String value;

    ASSERT_FALSE(m_db->HasProperty(DgnProjectProperty::ConnectedContextId())) << "Optional properties should not be present by default";

    ASSERT_EQ(BE_SQLITE_OK, m_db->SavePropertyString(DgnProjectProperty::ConnectedContextId(), fakeContextId1.ToString()));
    ASSERT_EQ(BE_SQLITE_ROW, m_db->QueryProperty(value, DgnProjectProperty::ConnectedContextId()));
    ASSERT_STREQ(value.c_str(), fakeContextId1.ToString().c_str()) << "Expected fakeContextId1";

    ASSERT_EQ(BE_SQLITE_OK, m_db->SavePropertyString(DgnProjectProperty::ConnectedContextId(), fakeContextId2.ToString()));
    ASSERT_EQ(BE_SQLITE_ROW, m_db->QueryProperty(value, DgnProjectProperty::ConnectedContextId()));
    ASSERT_STREQ(value.c_str(), fakeContextId2.ToString().c_str()) << "Expected fakeContextId2";

    ASSERT_EQ(BE_SQLITE_DONE, m_db->DeleteProperty(DgnProjectProperty::ConnectedContextId()));
    ASSERT_FALSE(m_db->HasProperty(DgnProjectProperty::ConnectedContextId())) << "Property should have been deleted";

    ASSERT_EQ(BE_SQLITE_OK, m_db->SavePropertyString(DgnProjectProperty::ConnectedContextId(), fakeContextId1.ToString()));
    ASSERT_EQ(BE_SQLITE_ROW, m_db->QueryProperty(value, DgnProjectProperty::ConnectedContextId()));
    ASSERT_STREQ(value.c_str(), fakeContextId1.ToString().c_str()) << "Expected fakeContextId1";
    }

/*---------------------------------------------------------------------------------**//**
* Schema Version can be accessed and it is correct
* @bsimethod                                    Majd.Uddin                   04/12
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
// @bsiclass                                                    Keith.Bentley   06/14
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
* @bsimethod                                    Majd.Uddin                   04/12
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
* @bsimethod                                    Algirdas.Mikoliunas                02/13
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
* @bsimethod                                    Adeel.Shoukat                      01/2013
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
* @bsimethod                                    Adeel.Shoukat                      01/2013
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
* @bsimethod                                  Ramanujam.Raman                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, CreateTrackedDgnDb)
{
    /*
     * Note: This test reproduces a reported problem in a very simple work flow of creating
     * a new DgnDb, turning it into a standalone briefcase, and then making some changes. The
     * temporary txn tables aren't setup properly for these cases and causes an exception.
     */

    DgnDomains::RegisterDomain(DgnPlatformTestDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);

    DbResult result = BE_SQLITE_ERROR;
    CreateDgnDbParams params(TEST_NAME);
    DgnDbPtr dgndb = DgnDb::CreateDgnDb(&result, DgnDbTestDgnManager::GetOutputFilePath(L"TrackedDb.ibim"), params);
    ASSERT_TRUE(dgndb.IsValid());

    SchemaStatus schemaStatus = DgnPlatformTestDomain::GetDomain().ImportSchema(*dgndb);
    ASSERT_TRUE(schemaStatus == SchemaStatus::Success);

    dgndb->SetAsBriefcase(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
    dgndb->Txns().EnableTracking(true);

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*dgndb, "TestPartition");
    ASSERT_TRUE(model.IsValid());

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*dgndb, "TestCategory");
    ASSERT_TRUE(categoryId.IsValid());

    CodeSpecId codeSpecId = DgnDbTestUtils::InsertCodeSpec(*dgndb, "TestCodeSpec");
    ASSERT_TRUE(codeSpecId.IsValid());

    CodeSpecCPtr codeSpec = dgndb->CodeSpecs().GetCodeSpec(codeSpecId);
    BeAssert(codeSpec.IsValid());

    DgnCode code = codeSpec->CreateCode("E1");

    TestElementPtr testel = TestElement::Create(*dgndb, model->GetModelId(), categoryId, code);
    testel->SetTestElementProperty("foo");
    DgnElementCPtr el = testel->Insert();
    ASSERT_TRUE(el.IsValid());

    dgndb->SaveChanges();

    // Check that we can turn the Briefcase -> Master
    BeGuid oldGuid = dgndb->QueryProjectGuid();
    result = dgndb->SetAsMaster();
    ASSERT_TRUE(result == BE_SQLITE_OK);
    BeGuid newGuid = dgndb->QueryProjectGuid();
    ASSERT_TRUE(oldGuid != newGuid && "A new GUID has to be assigned when turning a briefcase into a master copy");

    // Check that we can turn the Master -> Briefcase again
    result = dgndb->SetAsBriefcase(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
    ASSERT_TRUE(result == BE_SQLITE_OK);
    oldGuid = newGuid;
    newGuid = dgndb->QueryProjectGuid();
    ASSERT_TRUE(oldGuid == newGuid);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                  Ramanujam.Raman                 06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, SetUntrackedDbAsMaster)
{
    // TFS#905753 - Setup an untracked DB as a master copy after local changes
    DbResult result = BE_SQLITE_ERROR;
    CreateDgnDbParams params(TEST_NAME);
    DgnDbPtr dgndb = DgnDb::CreateDgnDb(&result, DgnDbTestDgnManager::GetOutputFilePath(L"MasterCopy.ibim"), params);
    ASSERT_TRUE(dgndb.IsValid());

    dgndb->SetAsBriefcase(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
    dgndb->Txns().EnableTracking(true);

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*dgndb, "TestPartition");
    ASSERT_TRUE(model.IsValid());

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*dgndb, "TestCategory");
    ASSERT_TRUE(categoryId.IsValid());

    dgndb->SaveChanges();
    dgndb->Txns().EnableTracking(false);

    // Check that we can turn the Briefcase -> Master
    result = dgndb->SetAsMaster();
    ASSERT_TRUE(result == BE_SQLITE_OK);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                  Ramanujam.Raman                 06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, ImportSchemaWithLocalChanges)
{
    // TFS#906843 was resolved as WAD. This ensures that that's really WAD - i.e.,
    // importing schema into a standalone briefcase with local changes should NOT be possible.
    DbResult result = BE_SQLITE_ERROR;
    CreateDgnDbParams params(TEST_NAME);
    DgnDbPtr dgndb = DgnDb::CreateDgnDb(&result, DgnDbTestDgnManager::GetOutputFilePath(L"ImportSchemaWithLocalChanges.ibim"), params);
    // Fails on Linux
    ASSERT_TRUE(dgndb.IsValid());

    dgndb->SetAsBriefcase(BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::Standalone()));
    dgndb->Txns().EnableTracking(true);

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
* @bsimethod                                    Adeel.Shoukat                      01/2013
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
* @bsimethod                                    Adeel.Shoukat                      01/2013
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
* @bsimethod                                    Adeel.Shoukat                      01/2013
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

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnProjectPackageTest : public DgnDbTestFixture
{
  public:
    //ScopedDgnHost m_autoDgnHost;

    /*---------------------------------------------------------------------------------**/ /**
        * @bsiclass                                            Julija.Suboc                08/13
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
        * @bsimethod                                            Julija.Suboc                08/13
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
        * @bsimethod                                            Julija.Suboc                08/13
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
        * @bsimethod                                            Julija.Suboc                08/13
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
        * @bsiclass                                            Julija.Suboc                08/13
        +---------------+---------------+---------------+---------------+---------------+------*/
    struct ProjectProperties
    {
        uint32_t modelCount;
        size_t spatialCategoryCount;
        size_t viewCount;
        size_t styleCount;
    };
    /*---------------------------------------------------------------------------------**/ /**
        * @bsimethod                                            Julija.Suboc                08/13
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
        * @bsimethod                                            Julija.Suboc                08/13
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
* @bsimethod                                            Julija.Suboc                08/13
* Creates package and checks if values from property table were saved correctly
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectPackageTest, CreatePackageUsingDefaults)
{
    //Collect properties for later verification
    DbResult status;
    SetupSeedProject();
    BeFileName testFile(m_db->GetDbFileName(), true);
    Utf8String testDatabase(testFile.GetFileNameAndExtension());
    PropertiesInTable propertiesInTable;
    getPropertiesInTable(m_db, propertiesInTable);
    m_db->CloseDb();
    //Create package and open it for verification
    BeFileName packageFile(testFile.GetDirectoryName());
    packageFile.AppendToPath(L"package.db");

    CreateIModelParams createParams;
    createParams.SetOverwriteExisting(true);
    status = DgnIModel::Create(packageFile, testFile, createParams);
    ASSERT_TRUE((int)DgnDbStatus::Success == status);
    BeSQLite::Db db;
    Db::OpenParams openParams(Db::OpenMode::Readonly);
    DbResult dbResult = db.OpenBeSQLiteDb(packageFile, openParams);
    ASSERT_TRUE(dbResult == BE_SQLITE_OK) << "Failed to open database with compressed file";
    //Check embedded files table and get file id
    DbEmbeddedFileTable &embeddedFiles = db.EmbeddedFiles();
    ASSERT_EQ(1, embeddedFiles.MakeIterator().QueryCount()) << "There should be only one embeded file";
    BeBriefcaseBasedId fileId = embeddedFiles.QueryFile(testDatabase.c_str());
    //Verify properties
    Utf8String propertToVerify;
    PropertiesInTable propertiesInTableV;
    getPackageProperties(db, propertiesInTableV, fileId.GetValue());
    compareProperties(propertiesInTable, propertiesInTableV);

    ASSERT_TRUE(DgnDbProfileVersion::GetCurrent().IsCurrent());
    ASSERT_FALSE(DgnDbProfileVersion::GetCurrent().IsPast());
    ASSERT_FALSE(DgnDbProfileVersion::GetCurrent().IsFuture());

    DgnDbProfileVersion fileVersion = DgnDbProfileVersion::Extract(testFile);
    ASSERT_TRUE(fileVersion.IsValid());
    ASSERT_TRUE(fileVersion.IsCurrent());

    DgnDbProfileVersion packageVersion = DgnDbProfileVersion::Extract(packageFile);
    ASSERT_TRUE(packageVersion.IsValid());
    ASSERT_TRUE(packageVersion.IsCurrent());

#if 0 // save for testing
    BeFileName oldBimFile(L"d:\\data\\dgndb\\imodel-generations\\Bim02_before_beProp_change.bim");
    DgnDbProfileVersion oldBimFileVersion = DgnDbProfileVersion::Extract(oldBimFile);
    ASSERT_FALSE(oldBimFileVersion.IsValid());

    {
    DbResult openStatus;
    DgnDbPtr oldBimDb = DgnDb::OpenDgnDb(&openStatus, oldBimFile, DgnDb::OpenParams(Db::OpenMode::Readonly));
    ASSERT_FALSE(oldBimDb.IsValid());
    ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooOld, openStatus);
    }

    DgnDbProfileVersion oldBimPackageVersion = DgnDbProfileVersion::Extract(BeFileName(L"d:\\data\\dgndb\\imodel-generations\\Bim02_before_beProp_change.imodel"));
    ASSERT_FALSE(oldBimPackageVersion.IsValid());

    BeFileName graphite05Package(L"d:\\data\\dgndb\\imodel-generations\\Hydrotreater Expansion_Graphite05.imodel");
    DgnDbProfileVersion versionGraphite05 = DgnDbProfileVersion::Extract(graphite05Package);
    ASSERT_TRUE(versionGraphite05.IsValid());
    ASSERT_TRUE(versionGraphite05.IsPast());
    ASSERT_TRUE(versionGraphite05.IsVersion_1_5());

    BeFileName dgnDb0601Package(L"d:\\data\\dgndb\\imodel-generations\\Hydrotreater Expansion_DgnDb0601.imodel");
    DgnDbProfileVersion versionDgnDb0601 = DgnDbProfileVersion::Extract(dgnDb0601Package);
    ASSERT_TRUE(versionDgnDb0601.IsValid());
    ASSERT_TRUE(versionDgnDb0601.IsPast());
    ASSERT_TRUE(versionDgnDb0601.IsVersion_1_6());

    BeFileName dgnDb0601File(L"d:\\data\\dgndb\\imodel-generations\\04_Plant_DgnDb0601Q4.idgndb");
    versionDgnDb0601 = DgnDbProfileVersion::Extract(dgnDb0601File);
    ASSERT_TRUE(versionDgnDb0601.IsValid());
    ASSERT_TRUE(versionDgnDb0601.IsPast());
    ASSERT_TRUE(versionDgnDb0601.IsVersion_1_6());

    DbResult openStatus;
    DgnDbPtr db0601 = DgnDb::OpenDgnDb(&openStatus, dgnDb0601File, DgnDb::OpenParams(Db::OpenMode::Readonly));
    ASSERT_FALSE(db0601.IsValid());
    ASSERT_EQ(BE_SQLITE_ERROR_ProfileTooOld, openStatus);
#endif
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                            Julija.Suboc                08/13
* Extracts file from package and checks if values in property table and some
* tables in file were extracted correctly. Covers ExtractFromPackage()
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectPackageTest, ExtractFromPackage)
{
    //Get some information which can be used later to verify if file was extracted correctly
    DbResult status;
    SetupSeedProject();
    BeFileName testFile(m_db->GetDbFileName(), true);
    WString testDatabase = testFile.GetFileNameAndExtension();
    //Get properties for later verification
    ProjectProperties projectProp;
    getProjectProperties(m_db, projectProp);
    PropertiesInTable properties;
    getPropertiesInTable(m_db, properties);
    m_db->CloseDb();
    //Create package
    BeFileName packageFile = DgnDbTestDgnManager::GetOutputFilePath(L"package.db");
    CreateIModelParams createParams;
    createParams.SetOverwriteExisting(true);
    status = DgnIModel::Create(packageFile, testFile, createParams);
    ASSERT_TRUE((int)DgnDbStatus::Success == status);
    //Extract package
    DbResult dbResult;
    //Prepare directory where file will be extracted
    BeFileName extractedFileDir;
    extractedFileDir.AppendToPath(DgnDbTestDgnManager::GetOutputFilePath(L"extractedFile"));
    if (!BeFileName::IsDirectory(extractedFileDir.GetName()))
        BeFileName::CreateNewDirectory(extractedFileDir.GetName());
    Utf8String fileName(testDatabase.c_str());
    auto filestat = DgnIModel::Extract(dbResult, extractedFileDir.GetNameUtf8().c_str(), fileName.c_str(), packageFile, true);
    EXPECT_EQ(DgnDbStatus::Success, filestat);
    EXPECT_TRUE(BE_SQLITE_OK == dbResult);
    //Open project that was extracted from package
    DgnDbPtr dgnProjV;
    BeFileName extractedFile(extractedFileDir.GetNameUtf8());
    extractedFile.AppendToPath(BeFileName::GetFileNameAndExtension(testFile.GetName()).c_str());
    DgnDbTestFixture::OpenDb(dgnProjV, extractedFile, BeSQLite::Db::OpenMode::ReadWrite);
    //Verify that properties did not change
    ProjectProperties projectPropV;
    getProjectProperties(dgnProjV, projectPropV);
    compareProjectProperties(projectProp, projectPropV);
    PropertiesInTable propertiesV;
    getPropertiesInTable(dgnProjV, propertiesV);
    compareProperties(properties, propertiesV);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                            Julija.Suboc                08/13
* Extracts file from package and checks if values in property table and some
* tables in file were extracted correctly. Covers ExtractFromPackageUsingDefault()
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectPackageTest, ExtractPackageUsingDefaults)
{
    //Open project
    DbResult status;
    SetupSeedProject();
    BeFileName testFile(m_db->GetDbFileName(), true);
    //Get properties for later verification
    ProjectProperties projectProp;
    getProjectProperties(m_db, projectProp);
    PropertiesInTable properties;
    getPropertiesInTable(m_db, properties);
    m_db->CloseDb();

    //Create package
    BeFileName packageFile = DgnDbTestDgnManager::GetOutputFilePath(L"package.db");
    CreateIModelParams createParams;
    createParams.SetOverwriteExisting(true);
    status = DgnIModel::Create(packageFile, testFile, createParams);
    ASSERT_TRUE((int)DgnDbStatus::Success == status);
    //Extract file from package
    DbResult dbResult;
    BeFileName extractedFile = DgnDbTestDgnManager::GetOutputFilePath(L"extractedUsingDefaults.ibim");
    auto fileStatus = DgnIModel::ExtractUsingDefaults(dbResult, extractedFile, packageFile, true);
    EXPECT_EQ(DgnDbStatus::Success, fileStatus);
    EXPECT_TRUE(BE_SQLITE_OK == dbResult);

    //Open file for verification
    DgnDbPtr dgnProjV;
    DgnDbTestFixture::OpenDb(dgnProjV, extractedFile, BeSQLite::Db::OpenMode::ReadWrite);
    //Verify that properties did not change
    ProjectProperties projectPropV;
    getProjectProperties(dgnProjV, projectPropV);
    compareProjectProperties(projectProp, projectPropV);
    PropertiesInTable propertiesV;
    getPropertiesInTable(dgnProjV, propertiesV);
    compareProperties(properties, propertiesV);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                             Muhammad Hassan                         07/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ImportSchema(ECSchemaR ecSchema, DgnDbR project)
{
    ECSchemaCachePtr schemaList = ECSchemaCache::Create();
    schemaList->AddSchema(ecSchema);
    return (SchemaStatus::Success == project.ImportSchemas(schemaList->GetSchemas())) ? SUCCESS : ERROR;
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                             Muhammad Hassan                         07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectPackageTest, EnforceLinkTableFor11Relationship)
{
    SetupSeedProject();

    BeFileName ecSchemaPath;
    BeTest::GetHost().GetDocumentsRoot(ecSchemaPath);
    ecSchemaPath.AppendToPath(L"DgnDb");
    ecSchemaPath.AppendToPath(L"Schemas");
    ecSchemaPath.AppendToPath(L"SampleDgnDbEditor.01.00.ecschema.xml");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater(m_db->GetSchemaLocater());
    WString schemaPath = BeFileName::GetDirectoryName(ecSchemaPath);
    schemaContext->AddSchemaPath(schemaPath.c_str());

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlFile(schema, ecSchemaPath, *schemaContext));
    ASSERT_TRUE(schema.IsValid());

    ASSERT_EQ(SchemaStatus::Success, m_db->ImportSchemas(schemaContext->GetCache().GetSchemas()));

    ASSERT_TRUE(m_db->TableExists("sdde_ArchStoreyWithElements"));
    ASSERT_TRUE(m_db->TableExists("sdde_ArchWithHVACStorey"));
}


struct ImportTests : DgnDbTestFixture
{
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
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

// Waiting for BisCore.01.00.06 to enable
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    08/2019
//---------------------------------------------------------------------------------------
//TEST_F(ImportTests, AssetSchemaImport)
//    {
//    Utf8CP schemaXml =
//        "<ECSchema schemaName=\"Asset\" alias=\"asset\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
//        "  <ECSchemaReference name='BisCore' version='01.00' alias='bis'/>"
//        "  <ECEntityClass typeName='AssetElement' modifier='Abstract'>"
//        "    <BaseClass>bis:RoleElement</BaseClass>"
//        "  </ECEntityClass>"
//        "  <ECEntityClass typeName='AssetModel'>"
//        "    <BaseClass>bis:RoleModel</BaseClass>"
//        "  </ECEntityClass>"
//        "</ECSchema>";
//
//    SetupSeedProject();
//    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
//    schemaContext->AddSchemaLocater(m_db->GetSchemaLocater());
//
//    ECSchemaPtr schema;
//    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
//    ASSERT_EQ(SchemaReadStatus::Success, schemaStatus);
//    }
