/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnProject_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

    static uint64_t GetCurrentTime() {return BeTimeUtilities::QueryMillisecondsCounter();}

    StepTimer(WCharCP msg) {m_msg=msg; m_startTime=GetCurrentTime();}
    ~StepTimer() {wprintf(L"%ls, %lf Seconds\n", m_msg, (GetCurrentTime()-m_startTime)/1000.);}
};
//=======================================================================================
// @bsiclass                                        Umar.Hayat              07/16
//=======================================================================================
struct DgnDbTest : public DgnDbTestFixture
{
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnDbTest, Settings)
    {
    Utf8CP val1 = "value 1";
    Utf8CP val2 = "value 2";
    Utf8CP val3 = "value 3";
    PropertySpec test1("test1", "testapp", PropertySpec::Mode::Setting);

    BeFileName projectName;

    if (true)
        {
        SetupSeedProject();
        DgnDbP newProject = m_db.get();
        ASSERT_TRUE( newProject != NULL );
    
        projectName.SetNameUtf8(newProject->GetDbFileName());

        // establish inititial value
        newProject->SavePropertyString(test1, val1);

        Utf8String val;
        newProject->QueryProperty(val, test1);         // make sure we can read it back
        ASSERT_TRUE (0 == strcmp(val.c_str(), val1));

        newProject->SavePropertyString(test1, val2);   // change the value
        newProject->QueryProperty(val, test1);         // and verify we get the new value back
        ASSERT_TRUE (0 == strcmp(val.c_str(), val2));

        newProject->SaveSettings();                     // save to file
        newProject->QueryProperty(val, test1);         // read it back
        ASSERT_TRUE (0 == strcmp(val.c_str(), val2));  // make sure it is still unchanged

        newProject->SavePropertyString(test1, val3);   // change it to a new value again
        newProject->QueryProperty(val, test1);
        ASSERT_TRUE (0 == strcmp(val.c_str(), val3));  // make sure we have new value in temp table.
        SaveDb();
        CloseDb();
        } // NOW - close the file. The setting should be saved persistently in its second state.

    // reopen the project
    DgnDbPtr sameProjectPtr;
    DgnDbTestFixture::OpenDb(sameProjectPtr, projectName, BeSQLite::Db::OpenMode::ReadWrite);
    ASSERT_TRUE( sameProjectPtr.IsValid());

    Utf8String val;
    sameProjectPtr->QueryProperty(val, test1);
    ASSERT_TRUE (0 == strcmp(val.c_str(), val2));      // make sure we get "value 2"

    sameProjectPtr->DeleteProperty(test1);              // delete the setting
    BeSQLite::DbResult rc = sameProjectPtr->QueryProperty(val, test1);    // verify we can't read it any more.
    ASSERT_NE (BE_SQLITE_ROW, rc);
    sameProjectPtr->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnDbTest, CheckStandardProperties)
    {
    SetupSeedProject();

    DgnDbP project = m_db.get();
    ASSERT_TRUE( project != NULL );
    Utf8String val;

    // Check that std properties are in the be_Props table. We can only check the value of a few using this API.
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("DbGuid",            "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("BeSQLiteBuild",     "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("CreationDate",      "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("Units",             "dgn_Proj"      )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("LastEditor",        "dgn_Proj"      )) );
    
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("SchemaVersion",     "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("SchemaVersion",     "ec_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("SchemaVersion",     "dgn_Proj"      )) );

    //  Use the model API to access model properties and check their values
    PhysicalModelPtr defaultModel = GetDefaultPhysicalModel();

    //  Use ModelInfo as an alt. way to get at some of the same property data
    GeometricModel::DisplayInfo const& displayInfo = defaultModel->GetDisplayInfo();
    ASSERT_TRUE(displayInfo.GetMasterUnits().GetBase() == UnitBase::Meter);
    ASSERT_TRUE(displayInfo.GetSubUnits().GetBase() == UnitBase::Meter);
    }

/*---------------------------------------------------------------------------------**//**
* Schema Version can be accessed and it is correct
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, ProjectSchemaVersions)
    {
    SetupSeedProject();
    DgnDbP project = m_db.get();
    ASSERT_TRUE( project != NULL);

    // Get Schema version details
    DgnVersion schemaVer = project->GetSchemaVersion();
    ASSERT_EQ (DGNDB_CURRENT_VERSION_Major, schemaVer.GetMajor()) << "The Schema Major Version is: " << schemaVer.GetMajor();
    ASSERT_EQ (DGNDB_CURRENT_VERSION_Minor, schemaVer.GetMinor()) << "The Schema Minor Version is: " << schemaVer.GetMinor();
    ASSERT_EQ (DGNDB_CURRENT_VERSION_Sub1, schemaVer.GetSub1()) << "The Schema Sub1 Version is: " << schemaVer.GetSub1();
    ASSERT_EQ (DGNDB_CURRENT_VERSION_Sub2, schemaVer.GetSub2()) << "The Schema Sub2 Version is: " << schemaVer.GetSub2();
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

/*---------------------------------------------------------------------------------**//**
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
    ASSERT_TRUE (project != NULL);
    ASSERT_EQ (BE_SQLITE_OK, status) << "Status returned is:" << status;

    //Create another project with same name. It should fail
    project2 = DgnDb::CreateDgnDb(&status2, DgnDbTestDgnManager::GetOutputFilePath(L"dup.ibim"), params);
    EXPECT_FALSE (project2.IsValid()) << "Project with Duplicate name should not be created";
    EXPECT_EQ (BE_SQLITE_ERROR_FileExists, status2) << "Status returned for duplicate name is: " << status2;
    }

/*---------------------------------------------------------------------------------**//**
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
    EXPECT_EQ (BE_SQLITE_OK, status1) << status1;
    ASSERT_TRUE (dgnProj1 != NULL);

    DbResult status2;
    DgnDbPtr dgnProj2;
    dgnProj2 = DgnDb::OpenDgnDb(&status2, testFile, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Exclusive));
    EXPECT_NE (BE_SQLITE_OK, status2) << status2;
    ASSERT_TRUE (dgnProj2 == NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, InvalidFileFormat)
    {
    DgnDbPtr      dgnProj;
    BeFileName path;
    StatusInt testDataFound = TestDataManager::FindTestData(path, L"ECSqlTest.01.00.ecschema.xml", BeFileName(L"DgnDb\\Schemas"));
    ASSERT_TRUE (SUCCESS == testDataFound);

    DbResult status;
    dgnProj = DgnDb::OpenDgnDb(&status, path, DgnDb::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ (BE_SQLITE_NOTADB, status) << status;
    ASSERT_TRUE( dgnProj == NULL);
    }

/*---------------------------------------------------------------------------------**//**
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
    ASSERT_TRUE( dgnProj != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, CreateWithInvalidName)
    {
    DgnDbPtr      dgnProj;

    BeFileName dgndbFileName;
    BeTest::GetHost().GetOutputRoot(dgndbFileName);
    dgndbFileName.AppendToPath(L"MyTextDGNDBFILE.txt");
    if (BeFileName::DoesPathExist(dgndbFileName))
        BeFileName::BeDeleteFile(dgndbFileName);

    DbResult status;
    CreateDgnDbParams params(TEST_NAME);
    dgnProj = DgnDb::CreateDgnDb(&status, BeFileName(dgndbFileName.GetNameUtf8().c_str()), params);
    EXPECT_EQ (BE_SQLITE_OK, status) << status;
    ASSERT_TRUE( dgnProj != NULL);
    /////////It creates a DgnDbfile with .txt extension having success status needs to figure out is this right behavior
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, FileNotFoundToOpen)
    {
    DbResult status;
    CreateDgnDbParams Obj;
    DgnDbPtr      dgnProj;

    BeFileName dgndbFileNotExist;
    BeTest::GetHost().GetOutputRoot(dgndbFileNotExist);
    dgndbFileNotExist.AppendToPath(L"MyFileNotExist.ibim");

    dgnProj = DgnDb::OpenDgnDb(&status, BeFileName(dgndbFileNotExist.GetNameUtf8().c_str()), DgnDb::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ (BE_SQLITE_ERROR_FileNotFound, status) << status;
    ASSERT_TRUE( dgnProj == NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, OpenAlreadyOpen)
    {
    BeFileName dgndbFileName;
    ASSERT_TRUE(DgnDbStatus::Success == DgnDbTestFixture::GetSeedDbCopy(dgndbFileName, L"OpenAlreadyOpen.bim"));

    DbResult status;
    DgnDbPtr dgnProj = DgnDb::OpenDgnDb(&status, dgndbFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Exclusive));
    EXPECT_EQ (BE_SQLITE_OK, status) << status;
    ASSERT_TRUE( dgnProj != NULL);

    // once a Db is opened for ReadWrite with exclusive access, it can't be opened, even for read.
    DgnDbPtr dgnProj1 = DgnDb::OpenDgnDb(&status, dgndbFileName, DgnDb::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ (BE_SQLITE_BUSY, status) << status;
    ASSERT_TRUE( dgnProj1 == NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Julija.Suboc                08/13
* Covers GetAzimuth(), GetLatitude() and GetLongitude() in DgnDb
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbTest, GetCoordinateSystemProperties)
    {
    SetupWithPrePublishedFile(L"GeoCoordinateSystem.i.ibim", L"GetCoordinateSystemProperties.ibim", BeSQLite::Db::OpenMode::Readonly);
    DgnGCS* dgnGCS = m_db->Units().GetDgnGCS();
    double azimuth = (dgnGCS != nullptr) ? dgnGCS->GetAzimuth() : 0.0;
    double azimuthExpected = 178.29138626108181;
    double eps = 0.0001;
    EXPECT_TRUE(fabs(azimuthExpected - azimuth) < eps) << "Expected different azimuth ";
    GeoPoint gorigin;
    m_db->Units().LatLongFromXyz(gorigin, DPoint3d::FromZero());
    double const latitudeExpected = 42.3413;
    EXPECT_TRUE(fabs(latitudeExpected - gorigin.latitude) < eps)<<"Expected diffrent latitude ";
    double const longitudeExpected = -71.0806;
    EXPECT_TRUE(fabs(longitudeExpected - gorigin.longitude) < eps)<<"Expected diffrent longitude ";
    }

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnProjectPackageTest : public DgnDbTestFixture
    {
     public:
        //ScopedDgnHost m_autoDgnHost;
        /*---------------------------------------------------------------------------------**//**
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
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                            Julija.Suboc                08/13
        +---------------+---------------+---------------+---------------+---------------+------*/
        void getPropertiesInTable(DgnDbPtr& project, PropertiesInTable& properties)
            {
            project->QueryProperty(properties.version, DgnProjectProperty::SchemaVersion());
            project->QueryProperty(properties.name, DgnProjectProperty::Name());
            project->QueryProperty(properties.description, DgnProjectProperty::Description());
            project->QueryProperty(properties.client, DgnProjectProperty::Client());
            project->QueryProperty(properties.lastEditor, DgnProjectProperty::LastEditor());
            project->QueryProperty(properties.creationDate, Properties::CreationDate());
            }
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                            Julija.Suboc                08/13
        +---------------+---------------+---------------+---------------+---------------+------*/
        void getPackageProperties(BeSQLite::Db& db, PropertiesInTable& properties, uint64_t embeddedFileId)
            {
            db.QueryProperty(properties.version, DgnEmbeddedProjectProperty::SchemaVersion(), embeddedFileId);
            db.QueryProperty(properties.name, DgnEmbeddedProjectProperty::Name(), embeddedFileId);
            db.QueryProperty(properties.description, DgnEmbeddedProjectProperty::Description(), embeddedFileId);
            db.QueryProperty(properties.client, DgnEmbeddedProjectProperty::Client(), embeddedFileId);
            db.QueryProperty(properties.lastEditor, DgnEmbeddedProjectProperty::LastEditor(), embeddedFileId);
            db.QueryProperty(properties.creationDate, DgnEmbeddedProjectProperty::CreationDate(), embeddedFileId);
            }
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                            Julija.Suboc                08/13
        +---------------+---------------+---------------+---------------+---------------+------*/
        void compareProperties(PropertiesInTable& prop, PropertiesInTable& propV)
            {
            EXPECT_TRUE(prop.version.CompareTo(propV.version)==0)<<"Version does not match";
            EXPECT_TRUE(prop.name.CompareTo(propV.name)==0)<<"Name does not match";
            EXPECT_TRUE(prop.description.CompareTo(propV.description)==0)<<"Description does not match";
            EXPECT_TRUE(prop.client.CompareTo(propV.client)==0)<<"Client does not match";
            EXPECT_TRUE(prop.lastEditor.CompareTo(propV.lastEditor)==0)<<"Last editor does not match";
            EXPECT_TRUE(prop.creationDate.CompareTo(propV.creationDate)==0)<<"Creation date does not match";
            }
        /*---------------------------------------------------------------------------------**//**
        * @bsiclass                                            Julija.Suboc                08/13
        +---------------+---------------+---------------+---------------+---------------+------*/
        struct ProjectProperties
            {
            uint32_t elmCount;
            uint32_t modelCount;
            size_t categoryCount;
            size_t viewCount;
            size_t styleCount;  
            };
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                            Julija.Suboc                08/13
        +---------------+---------------+---------------+---------------+---------------+------*/
        void getProjectProperties(DgnDbPtr& project, ProjectProperties& properties)
            {
            DgnModels& modelTable = project->Models();
            properties.elmCount = 0;
            properties.modelCount = 0;
            for (DgnModels::Iterator::Entry const& entry: modelTable.MakeIterator())
                {
                DgnModelPtr model = project->Models().GetModel(entry.GetModelId());
                model->FillModel();
                properties.elmCount += (int) model->GetElements().size();
                properties.modelCount++;
                }
            properties.viewCount = ViewDefinition::QueryCount(*project);
            properties.categoryCount = DgnCategory::QueryCount(*project);
#ifdef STYLE_REWRITE_06
            properties.styleCount = project->LineStyles().MakeIterator().QueryCount();
#else
            properties.styleCount = 0;
#endif
            }
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                            Julija.Suboc                08/13
        +---------------+---------------+---------------+---------------+---------------+------*/
        void compareProjectProperties(ProjectProperties& projProp, ProjectProperties& projPropV)
            {
            EXPECT_EQ(projProp.elmCount, projPropV.elmCount)<<"Element count does not match";
            EXPECT_EQ(projProp.modelCount, projPropV.modelCount)<<"Model count does not match";
            EXPECT_EQ(projProp.categoryCount, projPropV.categoryCount)<<"Category count does not match";
            EXPECT_EQ(projProp.viewCount, projPropV.viewCount)<<"View count does not match";
            EXPECT_EQ(projProp.styleCount, projPropV.styleCount)<<"Style count does not match";
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Julija.Suboc                08/13
* Creates package and checks if values from property table were saved correctly
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnProjectPackageTest, CreatePackageUsingDefaults)
    {
    //Collect properties for later verification
    DbResult status;
    SetupSeedProject();
    BeFileName testFile (m_db->GetDbFileName(),true);
    Utf8String testDatabase(testFile.GetFileNameAndExtension());
    PropertiesInTable propertiesInTable;
    getPropertiesInTable(m_db, propertiesInTable);
    m_db->CloseDb();
    //Create package and open it for verification
    BeFileName packageFile(testFile.GetDirectoryName());
    packageFile.AppendToPath(L"package.db");

    CreateIModelParams createParams;
    createParams.SetOverwriteExisting(true);
    status =  DgnIModel::Create(packageFile, testFile, createParams);
    ASSERT_TRUE ((int) DgnDbStatus::Success == status); 
    BeSQLite::Db        db;
    Db::OpenParams      openParams(Db::OpenMode::Readonly);
    DbResult dbResult = db.OpenBeSQLiteDb(packageFile, openParams);
    ASSERT_TRUE(dbResult == BE_SQLITE_OK)<<"Failed to open database with compressed file";
    //Check embedded files table and get file id
    DbEmbeddedFileTable& embeddedFiles = db.EmbeddedFiles();
    ASSERT_EQ(1, embeddedFiles.MakeIterator().QueryCount())<<"There should be only one embeded file";
    BeBriefcaseBasedId fileId = embeddedFiles.QueryFile(testDatabase.c_str());
    //Verify properties
    Utf8String propertToVerify;
    PropertiesInTable propertiesInTableV;
    getPackageProperties(db, propertiesInTableV, fileId.GetValue());
    compareProperties(propertiesInTable, propertiesInTableV);
    }

/*---------------------------------------------------------------------------------**//**
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
    status =  DgnIModel::Create(packageFile, testFile, createParams);
    ASSERT_TRUE ((int) DgnDbStatus::Success == status); 
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

/*---------------------------------------------------------------------------------**//**
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
    status =  DgnIModel::Create(packageFile, testFile, createParams);
    ASSERT_TRUE ((int) DgnDbStatus::Success == status); 
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         07/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ImportSchema(ECSchemaR ecSchema, DgnDbR project)
    {
    ECSchemaCachePtr schemaList = ECSchemaCache::Create();
    schemaList->AddSchema(ecSchema);
    return project.Schemas().ImportECSchemas(schemaList->GetSchemas());
    }

/*---------------------------------------------------------------------------------**//**
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

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    WString schemaPath = BeFileName::GetDirectoryName(ecSchemaPath);
    schemaContext->AddSchemaPath(schemaPath.c_str());

    ECSchemaPtr schema;
    ECSchema::ReadFromXmlFile(schema, ecSchemaPath, *schemaContext);
    ASSERT_TRUE(schema.IsValid());

    ASSERT_EQ(SUCCESS, ImportSchema(*schema, *m_db));

    ASSERT_TRUE(m_db->TableExists("sdde_ArchStoreyWithElements"));
    ASSERT_TRUE(m_db->TableExists("sdde_ArchWithHVACStorey"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
TEST(DgnProject, DuplicateElementId)
    {
    // // TFS#289237 stems from platform re-using a deleted element ID.
    // // Before the patch, DgnFile only read/cached the highest element ID when asked for it.
    // // Thus, if the first thing you did was delete the element with the highest ID, you could end up adding a new elenent with the same ID, and various chaos ensues.
    // // This test is meant to verify the fix that the highest ID is read when the file is opened, instead of later when adding elements.

    // ScopedDgnHost host;
    
    // // We're going to write to the file; make a copy.
    // BeFileName dbPath;
    // ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(dbPath, L"3dMetricGeneral.ibim", L"DgnProject.DuplicateElementId.bim", __FILE__));
    
    // DgnModelId modelId;
    // ElementId firstAddId;

    // // Open the file.
    //     {
    //     DgnFileStatus openStatus;
    //     DgnProjectPtr db = DgnProject::OpenProject (&openStatus, dbPath, DgnProject::OpenParams(DgnProject::OPEN_ReadWrite));
    //     ASSERT_TRUE(DGNFILE_STATUS_Success == openStatus);
    //     ASSERT_TRUE(db.IsValid());

    //     // Find a model to work with.
    //     DgnModels::Iterator modelIter = db->Models().MakePhysicalIterator();
    //     ASSERT_TRUE(modelIter.begin() != modelIter.end());
        
    //     modelId = modelIter.begin().GetModelId();
    //     ASSERT_TRUE(modelId.IsValid());
        
    //     DgnModelP model = db->Models().GetModelById(modelId);
    //     ASSERT_TRUE(nullptr != model);
    //     ASSERT_TRUE(SUCCESS == model->FillSections(DgnModelSections::All));

    //     // Add an element, and remember its ID.
    //     EditElementHandle eeh;
    //     ExtendedElementHandler::InitializeElement(eeh, nullptr /*template*/, model, model->Is3d() /*is3d*/, false /*isComplexHeader*/);
    //     ASSERT_TRUE(eeh.IsValid());
        
    //     ASSERT_TRUE(SUCCESS == eeh.AddToModel());
        
    //     firstAddId = eeh.GetElementId();
    //     ASSERT_TRUE(firstAddId.IsValid());
    //     }

    // // Re-open, delete the added element, and add another. Verify the ID was not re-used.
    //     {
    //     DgnFileStatus openStatus;
    //     DgnProjectPtr db = DgnProject::OpenProject (&openStatus, dbPath, DgnProject::OpenParams(DgnProject::OPEN_ReadWrite));
    //     ASSERT_TRUE(DGNFILE_STATUS_Success == openStatus);
    //     ASSERT_TRUE(db.IsValid());

    //     DgnModelP model = db->Models().GetModelById(modelId);
    //     ASSERT_TRUE(nullptr != model);
    //     ASSERT_TRUE(SUCCESS == model->FillSections(DgnModelSections::All));

    //     PersistentElementRefP firstAddElRef = model->FindElementById(firstAddId);
    //     ASSERT_TRUE(nullptr != firstAddElRef);

    //     EditElementHandle eeh(firstAddElRef);
    //     ASSERT_TRUE(SUCCESS == eeh.DeleteFromModel());

    //     EditElementHandle eeh2;
    //     ExtendedElementHandler::InitializeElement(eeh2, nullptr /*template*/, model, model->Is3d() /*is3d*/, false /*isComplexHeader*/);
    //     ASSERT_TRUE(eeh2.IsValid());
        
    //     ASSERT_TRUE(SUCCESS == eeh2.AddToModel());
    //     ElementId secondAddId = eeh2.GetElementId();
    //     ASSERT_TRUE(secondAddId.IsValid());
    //     ASSERT_TRUE(secondAddId > firstAddId);
    //     }
    }

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct ElementUriTests : ::testing::Test
{
    static void SetUpTestCase();
    static void TearDownTestCase();

    ScopedDgnHost m_host;
    RefCountedPtr<Dgn::NamespaceAuthority> m_codeAuthority;

    static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;

    ElementUriTests()
        {
        // Must register my domain whenever I initialize a host
        DgnPlatformTestDomain::Register();
        }

    NamespaceAuthority& GetTestCodeAuthority(DgnDbR db)
        {
        if (!m_codeAuthority.IsValid())
            {
            m_codeAuthority = NamespaceAuthority::CreateNamespaceAuthority("TestAuthority", db);
            DgnDbStatus status = m_codeAuthority->Insert();
            BeAssert(status == DgnDbStatus::Success);
            }
        return *m_codeAuthority;
        }

    DgnCode CreateCode(DgnDbR db, Utf8CP ns, Utf8CP elementCode)
        {
        return GetTestCodeAuthority(db).CreateCode(elementCode, ns);
        }

};

DgnPlatformSeedManager::SeedDbInfo ElementUriTests::s_seedFileInfo;

//---------------------------------------------------------------------------------------
// Do one-time setup for all tests in this group
// In this case, I just request the (root) seed file that my tests will use and make a note of it.
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void ElementUriTests::SetUpTestCase() 
    {
    ScopedDgnHost tempHost;
    ElementUriTests::s_seedFileInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(true, true));
    }

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void ElementUriTests::TearDownTestCase()
    {
    // Note: leave your subdirectory in place. Don't remove it. That allows the 
    // base class to detect and throw an error if two groups try to use a directory of the same name.
    // Don't worry about stale data. The test runner will clean out everything at the start of the program.
    // You can empty the directory, if you want to save space.
    //DgnPlatformSeedManager::EmptySubDirectory(GROUP_SUBDIR);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             08/2016
//---------------------------------------------------------------------------------------
TEST_F(ElementUriTests, Test1)
    {
    // Note: We know that our group's TC_SETUP function has already created the group seed file. We can just ask for it.
    DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(s_seedFileInfo.fileName, L"Test1");
    ASSERT_TRUE(db.IsValid());

    DgnModelId mid = db->Models().QueryModelId(s_seedFileInfo.modelCode);
    DgnCategoryId catId = DgnCategory::QueryCategoryId(s_seedFileInfo.categoryName, *db);

    DgnElementCPtr el;
    DgnElementCPtr elNoProps;
    if (true)
        {
        TestElementPtr testel = TestElement::Create(*db, mid, catId, CreateCode(*db, "TestNS", "E1"));
        testel->SetTestElementProperty("foo");
        el = testel->Insert();
        ASSERT_TRUE(el.IsValid());

        elNoProps = TestElement::Create(*db, mid, catId)->Insert();;
        ASSERT_TRUE(elNoProps.IsValid());

        db->SaveChanges();
        }

    if (true)
        {
        // Graphite format - ECClass=
        Utf8CP uri = "/DgnElements?ECClass=DgnPlatformTest:TestElement&TestElementProperty=foo";
        auto eid = db->Elements().QueryElementIdByURI(uri);
        ASSERT_TRUE(eid == el->GetElementId());

        Utf8CP baduri = "/DgnElements?ECClass=DgnPlatformTest:TestElement&TestElementProperty=bar";
        auto badeid = db->Elements().QueryElementIdByURI(baduri);
        ASSERT_TRUE(!badeid.IsValid());
        }

    if (true)
        {
        // DgnDb format - ECCass=
        Utf8CP uri = "/DgnDb?ECClass=DgnPlatformTest:TestElement&TestElementProperty=foo";
        auto eid = db->Elements().QueryElementIdByURI(uri);
        ASSERT_TRUE(eid == el->GetElementId());

        Utf8CP baduri = "/DgnDb?ECClass=DgnPlatformTest:TestElement&TestElementProperty=bar";
        auto badeid = db->Elements().QueryElementIdByURI(baduri);
        ASSERT_TRUE(!badeid.IsValid());
        }

    if (true)
        {
        // DgnDb format - Code

        Utf8String dbUri;
        ASSERT_EQ(BSISUCCESS, db->Elements().CreateElementUri(dbUri, *el, true, true));
        ASSERT_TRUE(el->GetElementId() == db->Elements().QueryElementIdByURI(dbUri.c_str()));

        ASSERT_TRUE(el->GetElementId() == db->Elements().QueryElementIdByURI("/DgnDb?Code=E1&A=TestAuthority&N=TestNS"));

        BeTest::SetFailOnAssert(false);
        ASSERT_TRUE(!db->Elements().QueryElementIdByURI("/DgnDb?CodXYZ=E1&A=TestAuthority&N=TestNS").IsValid());
        BeTest::SetFailOnAssert(true);
        }


    if (true)
        {
        // fallBackOnDgnDbId
        Utf8String dbUri;
        ASSERT_EQ(BSISUCCESS, db->Elements().CreateElementUri(dbUri, *elNoProps, true, true));
        ASSERT_TRUE(Utf8String::npos == dbUri.find('?')) << " This URI should not contain a query (just an ID)";
        ASSERT_TRUE(elNoProps->GetElementId() == db->Elements().QueryElementIdByURI(dbUri.c_str()));
        }

    }

#ifdef WIP_HAVE_LEGACY_FILES
//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             08/2016
//---------------------------------------------------------------------------------------
static void testLegacyUri(DgnDbR db, Utf8CP uriQuery)
    {
    // Make sure we can resolve a legacy Graphite URI ...
    Utf8String graphiteUri("/DgnElements?");
    graphiteUri.append(uriQuery);
    auto eid1 = db.Elements().QueryElementIdByURI(graphiteUri.c_str());
    ASSERT_TRUE(eid1.IsValid());

    auto el = db.Elements().GetElement(eid1);;
    ASSERT_TRUE(el.IsValid());

    // ... and that we can create a similar URI to the same element in the new version
    Utf8String uri1;
    ASSERT_EQ(BSISUCCESS, db.Elements().CreateElementUri(uri1, *el, false, false));
    ASSERT_TRUE(uri1.substr(0,7) == "/DgnDb?");
    // Compare the query part (leaving out the path /DgnDb? and /DgnElements?)
    ASSERT_TRUE(uri1.substr(7) == uriQuery);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                         Ramanujam.Raman             08/2016
//---------------------------------------------------------------------------------------
static void testProvenanceUri(DgnDbR db, Utf8CP uri)
    {
    // Test URI query in (potentially) old version
    DgnElementId eid1 = db.Elements().QueryElementIdByURI(uri);
    ASSERT_TRUE(eid1.IsValid());

    DgnElementCPtr el = db.Elements().GetElement(eid1);;
    ASSERT_TRUE(el.IsValid());

    // Test URI creation in the latest version
    Utf8String dgndbUri;
    ASSERT_EQ(BSISUCCESS, db.Elements().CreateElementUri(dgndbUri, *el, true, false));
    DgnElementId eid2 = db.Elements().QueryElementIdByURI(dgndbUri.c_str());

    ASSERT_TRUE(eid1 == eid2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             08/2016
//---------------------------------------------------------------------------------------
TEST_F(ElementUriTests, TestLegacyUris)
    {
    DbResult openStatus;
    DgnDb::OpenParams openParams(DgnDb::OpenMode::Readonly);
    DgnDbPtr db;
    
    db = DgnDb::OpenDgnDb(&openStatus, BeFileName(L"d:\\tmp\\Office Building.i.ibim"), openParams);
    ASSERT_TRUE(db.IsValid());
    testLegacyUri(*db, "ECClass=Bentley%5FRevit%5FSchema%3APlanting&RevitConnectorID=513243");
    testLegacyUri(*db, "ECClass=Bentley%5FRevit%5FSchema%3ARoofs&RevitConnectorID=343500");
    testProvenanceUri(*db, "/DgnElements?SourceId=Civil%2Edgn%2Ei%2Edgn&ElementId=8512");
    testProvenanceUri(*db, "/DgnDb?FileName=office%20building%2Ei%2Edgn%3C2%3Ecivil%2Edgn%2Ei%2Edgn&ElementId=8512");

    db = DgnDb::OpenDgnDb(&openStatus, BeFileName(L"d:\\tmp\\Hydrotreater Expansion.i.ibim"), openParams);
    ASSERT_TRUE(db.IsValid());
    testLegacyUri(*db, "ECClass=OpenPlant%5F3D%3ASHELL%5FAND%5FTUBE%5FHEAT%5FEXCHANGER%5FPAR&GUID=A1AC9BB1%2D9AAF%2D4A9D%2D96B6%2DD5C242358405");
    testLegacyUri(*db, "ECClass=OpenPlant%5F3D%3ASTORAGE%5FTANK%5FPAR&GUID=D3F923AD%2D5694%2D4300%2DBE23%2D1953D6D0D02B");
    testProvenanceUri(*db, "/DgnElements?SourceId=Civil%2Edgn%2Ei%2Edgn&ElementId=5029");
    testProvenanceUri(*db, "/DgnElements?SourceId=S%5FExhaust%2Edgn%2Ei%2Edgn&ElementId=1971");

    if (true)
        {
        db = DgnDb::OpenDgnDb(&openStatus, BeFileName(L"d:\\tmp\\BGRSmall976.bim"), openParams);
        ASSERT_TRUE(db.IsValid());

        auto eid1 = db->Elements().QueryElementIdByURI("/DgnDb?Code=29V%252D9&A=ConstructionPlanning%255FPhysicalHierarchy&N=Equipment");
        ASSERT_TRUE(eid1.IsValid());
        
        auto eid2 = db->Elements().QueryElementIdByURI("/DgnDb?Code=CB1002%252D0%252D6420&A=ConstructionPlanning%255FPhysicalHierarchy&N=Piping%252F03A%252D45405%252D01%252FSPOOL%252003A%252D45405%252D01%252D3");
        ASSERT_TRUE(eid2.IsValid());
        }
    }
#endif

struct ImportTests : DgnDbTestFixture
    {};

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

    ASSERT_EQ(DgnDbStatus::Success, BisCoreDomain::GetDomain().ImportSchema(*m_db, schemaContext->GetCache()));
    ASSERT_TRUE(m_db->IsDbOpen());
    }
