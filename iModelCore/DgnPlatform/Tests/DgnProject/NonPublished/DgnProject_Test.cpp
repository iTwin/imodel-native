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
    WCharCP m_msg;
    uint64_t m_startTime;

    static uint64_t GetCurrentTime() {return BeTimeUtilities::QueryMillisecondsCounter();}

    StepTimer(WCharCP msg) {m_msg=msg; m_startTime=GetCurrentTime();}
    ~StepTimer() {wprintf(L"%ls, %lf Seconds\n", m_msg, (GetCurrentTime()-m_startTime)/1000.);}
};

static void openProject(DgnDbPtr& project, BeFileName const& projectFileName, Db::OpenMode mode = Db::OpenMode::Readonly)
    {
    DbResult result;
    project = DgnDb::OpenDgnDb(&result, projectFileName, DgnDb::OpenParams(mode));

    ASSERT_TRUE( project != NULL);
    ASSERT_TRUE( result == BE_SQLITE_OK );

    Utf8String projectFileNameUtf8(projectFileName.GetName());
    ASSERT_TRUE( projectFileNameUtf8 == Utf8String(project->GetDbFileName()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DgnDb, Settings)
    {
    ScopedDgnHost autoDgnHost;

    Utf8CP val1 = "value 1";
    Utf8CP val2 = "value 2";
    Utf8CP val3 = "value 3";
    PropertySpec test1("test1", "testapp", PropertySpec::Mode::Setting);

    BeFileName projectName;

    if (true)
        {
        DgnDbTestDgnManager tdm(L"2dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite, false);
        DgnDbP newProject = tdm.GetDgnProjectP();
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
        } // NOW - close the file. The setting should be saved persistently in its second state.

    // reopen the project
    DgnDbPtr sameProjectPtr;
    openProject(sameProjectPtr, projectName, BeSQLite::Db::OpenMode::ReadWrite);
    ASSERT_TRUE( sameProjectPtr.IsValid());

    Utf8String val;
    sameProjectPtr->QueryProperty(val, test1);
    ASSERT_TRUE (0 == strcmp(val.c_str(), val2));      // make sure we get "value 2"

    sameProjectPtr->DeleteProperty(test1);              // delete the setting
    BeSQLite::DbResult rc = sameProjectPtr->QueryProperty(val, test1);    // verify we can't read it any more.
    ASSERT_NE (BE_SQLITE_ROW, rc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DgnDb, CheckStandardProperties)
    {
    ScopedDgnHost autoDgnHost;

    //DbResult rc;
    Utf8String val;

    DgnDbTestDgnManager tdm(L"2dMetricGeneral.idgndb", __FILE__, Db::OpenMode::Readonly, false);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    // Check that std properties are in the be_Props table. We can only check the value of a few using this API.
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("DbGuid",            "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("BeSQLiteBuild",     "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("CreationDate",      "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("Description",       "dgn_Proj"      )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("Units",             "dgn_Proj"      )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("LastEditor",        "dgn_Proj"      )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("DefaultView",       "dgn_View"      )) );
    
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("SchemaVersion",     "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("SchemaVersion",     "ec_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty(val, PropertySpec("SchemaVersion",     "dgn_Proj"      )) );

    //  Use the model API to access model properties and check their values
    DgnModelPtr defaultModel = project->Models().GetModel(project->Models().QueryFirstModelId());

    //  Use ModelInfo as an alt. way to get at some of the same property data
    GeometricModel::DisplayInfo const& displayInfo = defaultModel->ToGeometricModel()->GetDisplayInfo();
    ASSERT_TRUE(displayInfo.GetMasterUnits().GetBase() == UnitBase::Meter);
    ASSERT_TRUE(displayInfo.GetSubUnits().GetBase() == UnitBase::Meter);
    }

/*---------------------------------------------------------------------------------**//**
* Schema Version can be accessed and it is correct
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnDb, ProjectSchemaVersions)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite, TestDgnManager::DGNINITIALIZEMODE_None);
    DgnDbP project = tdm.GetDgnProjectP();
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
TEST(DgnDb, ProjectWithDuplicateName)
    {
    ScopedDgnHost autoDgnHost;

    CreateDgnDbParams params;
    params.SetOverwriteExisting(false);
    DbResult status, status2;
    DgnDbPtr project, project2;
    
    //Deleting the project file if it exists already
    BeFileName::BeDeleteFile(DgnDbTestDgnManager::GetOutputFilePath(L"dup.idgndb"));

    //Create and Verify that project was created
    project = DgnDb::CreateDgnDb(&status, DgnDbTestDgnManager::GetOutputFilePath(L"dup.idgndb"), params);
    ASSERT_TRUE (project != NULL);
    ASSERT_EQ (BE_SQLITE_OK, status) << "Status returned is:" << status;

    //Create another project with same name. It should fail
    project2 = DgnDb::CreateDgnDb(&status2, DgnDbTestDgnManager::GetOutputFilePath(L"dup.idgndb"), params);
    EXPECT_FALSE (project2.IsValid()) << "Project with Duplicate name should not be created";
    EXPECT_EQ (BE_SQLITE_ERROR_FileExists, status2) << "Status returned for duplicate name is: " << status2;
    }

/*---------------------------------------------------------------------------------**//**
* Try multiple read/write for project
* @bsimethod                                    Algirdas.Mikoliunas                02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnDb, MultipleReadWrite)
    {
    ScopedDgnHost autoDgnHost;

    BeFileName fullFileName;
    TestDataManager::FindTestData(fullFileName, L"ElementsSymbologyByLevel.idgndb", DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    
    BeFileName testFile = DgnDbTestDgnManager::GetOutputFilePath(L"ElementsSymbologyByLevel.idgndb");
    BeFileName::BeCopyFile(fullFileName, testFile);

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
TEST(DgnDb, InvalidFileFormat)
    {
    ScopedDgnHost           autoDgnHost;
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
TEST(DgnDb, CreateDgnDb)
    {
    ScopedDgnHost           autoDgnHost;
    DgnDbPtr      dgnProj;
    BeFileName dgndbFileName;
    BeTest::GetHost().GetOutputRoot(dgndbFileName);
    dgndbFileName.AppendToPath(L"MyFile.idgndb");

     if (BeFileName::DoesPathExist(dgndbFileName))
        BeFileName::BeDeleteFile(dgndbFileName);
    DbResult status;
    CreateDgnDbParams Obj;
    dgnProj = DgnDb::CreateDgnDb(&status, BeFileName(dgndbFileName.GetNameUtf8().c_str()),Obj);
    EXPECT_EQ (BE_SQLITE_OK, status) << status;
    ASSERT_TRUE( dgnProj != NULL);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnDb, CreateWithInvalidName)
    {
    ScopedDgnHost           autoDgnHost;
    DgnDbPtr      dgnProj;

    BeFileName dgndbFileName;
    BeTest::GetHost().GetOutputRoot(dgndbFileName);
    dgndbFileName.AppendToPath(L"MyTextDGNDBFILE.txt");
    if (BeFileName::DoesPathExist(dgndbFileName))
        BeFileName::BeDeleteFile(dgndbFileName);

    DbResult status;
    CreateDgnDbParams Obj;
    dgnProj = DgnDb::CreateDgnDb(&status, BeFileName(dgndbFileName.GetNameUtf8().c_str()),Obj);
    EXPECT_EQ (BE_SQLITE_OK, status) << status;
    ASSERT_TRUE( dgnProj != NULL);
    /////////It creates a DgnDbfile with .txt extension haveing success status needs to figure out is this right behave
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnDb, FileNotFoundToOpen)
    {
    ScopedDgnHost autoDgnHost;
    DbResult status;
    CreateDgnDbParams Obj;
    DgnDbPtr      dgnProj;

    BeFileName dgndbFileNotExist;
    BeTest::GetHost().GetOutputRoot(dgndbFileNotExist);
    dgndbFileNotExist.AppendToPath(L"MyFileNotExist.idgndb");

    dgnProj = DgnDb::OpenDgnDb(&status, BeFileName(dgndbFileNotExist.GetNameUtf8().c_str()), DgnDb::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ (BE_SQLITE_ERROR_FileNotFound, status) << status;
    ASSERT_TRUE( dgnProj == NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnDb, OpenAlreadyOpen)
{
    ScopedDgnHost  autoDgnHost;

    WCharCP testFileName = L"ElementsSymbologyByLevel.idgndb";
    BeFileName sourceFile = DgnDbTestDgnManager::GetSeedFilePath(testFileName);

    BeFileName dgndbFileName;
    BeTest::GetHost().GetOutputRoot(dgndbFileName);
    dgndbFileName.AppendToPath(testFileName);

    BeFileName::BeCopyFile(sourceFile, dgndbFileName, false);

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
TEST(DgnDb, GetCoordinateSystemProperties)
    {
    ScopedDgnHost autoDgnHost;
    BeFileName fullFileName;
    TestDataManager::FindTestData(fullFileName, L"GeoCoordinateSystem.i.idgndb", DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    //Open project
    DgnDbPtr dgnProj;
    openProject(dgnProj, fullFileName, BeSQLite::Db::OpenMode::Readonly);
    DgnGCS* dgnGCS = dgnProj->Units().GetDgnGCS();
    double azimuth = (dgnGCS != nullptr) ? dgnGCS->GetAzimuth() : 0.0;
    double azimuthExpected = 178.2912;
    double eps = 0.0001;
    EXPECT_TRUE(fabs(azimuthExpected - azimuth) < eps) << "Expected different azimuth ";
    GeoPoint gorigin;
    dgnProj->Units().LatLongFromXyz(gorigin, DPoint3d::FromZero());
    double const latitudeExpected = 42.3413;
    EXPECT_TRUE(fabs(latitudeExpected - gorigin.latitude) < eps)<<"Expected diffrent latitude ";
    double const longitudeExpected = -71.0806;
    EXPECT_TRUE(fabs(longitudeExpected - gorigin.longitude) < eps)<<"Expected diffrent longitude ";
    }

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnProjectPackageTest : public testing::Test
    {
     public:
        ScopedDgnHost m_autoDgnHost;
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
            DgnStyles& styleTable = project->Styles();
            properties.styleCount = styleTable.LineStyles().MakeIterator().QueryCount();
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
    //Copy file to temp directory
    BeFileName fullFileName;
    TestDataManager::FindTestData(fullFileName, L"ElementsSymbologyByLevel.idgndb", DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    BeFileName testFile = DgnDbTestDgnManager::GetOutputFilePath(L"ElementsSymbologyByLevel.idgndb");
    BeFileName::BeCopyFile(fullFileName, testFile);
    //Collect properties for later verification
    DbResult status;
    DgnDbPtr dgnProj;
    openProject(dgnProj, testFile, BeSQLite::Db::OpenMode::ReadWrite);
    PropertiesInTable propertiesInTable;
    getPropertiesInTable(dgnProj, propertiesInTable);
    dgnProj->CloseDb();
    //Create package and open it for verification
    BeFileName packageFile = DgnDbTestDgnManager::GetOutputFilePath(L"package.db");
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
    BeBriefcaseBasedId fileId = embeddedFiles.QueryFile("ElementsSymbologyByLevel.idgndb");
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
    WCharCP testDatabase = L"ElementsSymbologyByLevel.idgndb";
    //Copy file to temp directory
    BeFileName fullFileName;
    TestDataManager::FindTestData(fullFileName, testDatabase, DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    BeFileName testFile = DgnDbTestDgnManager::GetOutputFilePath(L"ElementsSymbologyByLevel.idgndb");
    BeFileName::BeCopyFile(fullFileName, testFile);
    //Get some information which can be used later to verify if file was extracted correctly
    DbResult status;
    DgnDbPtr dgnProj;
    openProject(dgnProj, testFile, BeSQLite::Db::OpenMode::ReadWrite);
    //Get properties for later verification
    ProjectProperties projectProp;
    getProjectProperties(dgnProj, projectProp);
    PropertiesInTable properties;
    getPropertiesInTable(dgnProj, properties);
    dgnProj->CloseDb();
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
    Utf8String fileName;
    BeStringUtilities::WCharToUtf8(fileName, testDatabase);
    auto filestat = DgnIModel::Extract(dbResult, extractedFileDir.GetNameUtf8().c_str(), fileName.c_str(), packageFile, true);
    EXPECT_EQ(DgnDbStatus::Success, filestat);
    EXPECT_TRUE(BE_SQLITE_OK == dbResult);
    //Open project that was extracted from package  
    DgnDbPtr dgnProjV;
    BeFileName extractedFile(extractedFileDir.GetNameUtf8());
    extractedFile.AppendToPath(BeFileName::GetFileNameAndExtension(testFile.GetName()).c_str());
    openProject(dgnProjV, extractedFile, BeSQLite::Db::OpenMode::ReadWrite);
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
    //Copy project to temp folder
    WCharCP fileName = L"ElementsSymbologyByLevel.idgndb";
    BeFileName fullFileName;
    TestDataManager::FindTestData(fullFileName, fileName, DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    BeFileName testFile = DgnDbTestDgnManager::GetOutputFilePath(fileName);
    ASSERT_TRUE(BeFileNameStatus::Success ==BeFileName::BeCopyFile(fullFileName, testFile))<<"Failed to copy file";
    //Open project
    DbResult status;
    DgnDbPtr dgnProj;
    openProject(dgnProj, testFile, BeSQLite::Db::OpenMode::ReadWrite);
    //Get properties for later verification
    ProjectProperties projectProp;
    getProjectProperties(dgnProj, projectProp);
    PropertiesInTable properties;
    getPropertiesInTable(dgnProj, properties);
    dgnProj->CloseDb();
    
    //Create package
    BeFileName packageFile = DgnDbTestDgnManager::GetOutputFilePath(L"package.db");
    CreateIModelParams createParams;
    createParams.SetOverwriteExisting(true);
    status =  DgnIModel::Create(packageFile, testFile, createParams);
    ASSERT_TRUE ((int) DgnDbStatus::Success == status); 
    //Extract file from package
    DbResult dbResult;
    BeFileName extractedFile = DgnDbTestDgnManager::GetOutputFilePath(L"extractedUsingDefaults.idgndb");
    auto fileStatus = DgnIModel::ExtractUsingDefaults(dbResult, extractedFile, packageFile, true);
    EXPECT_EQ(DgnDbStatus::Success, fileStatus);
    EXPECT_TRUE(BE_SQLITE_OK == dbResult);
   
    //Open file for verification
    DgnDbPtr dgnProjV;
    openProject(dgnProjV, extractedFile, BeSQLite::Db::OpenMode::ReadWrite);
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
BentleyStatus ImportSchema (ECSchemaR ecSchema, DgnDbR project)
    {
    ECSchemaCachePtr schemaList = ECSchemaCache::Create ();
    schemaList->AddSchema (ecSchema);
    return project.Schemas ().ImportECSchemas (*schemaList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnProjectPackageTest, EnforceLinkTableFor11Relationship)
    {
    WCharCP testFileName = L"2dMetricGeneral.idgndb";
    BeFileName sourceFile = DgnDbTestDgnManager::GetSeedFilePath (testFileName);

    BeFileName dgndbFileName;
    BeTest::GetHost ().GetOutputRoot (dgndbFileName);
    dgndbFileName.AppendToPath (testFileName);

    ASSERT_EQ (BeFileNameStatus::Success, BeFileName::BeCopyFile (sourceFile, dgndbFileName, false));

    DbResult status;
    DgnDbPtr dgnProj = DgnDb::OpenDgnDb (&status, dgndbFileName, DgnDb::OpenParams (Db::OpenMode::ReadWrite));
    EXPECT_EQ (DbResult::BE_SQLITE_OK, status) << status;
    ASSERT_TRUE (dgnProj != NULL);

    BeFileName ecSchemaPath;
    BeTest::GetHost ().GetDocumentsRoot (ecSchemaPath);
    ecSchemaPath.AppendToPath (L"DgnDb");
    ecSchemaPath.AppendToPath (L"Schemas");
    ecSchemaPath.AppendToPath (L"SampleDgnDbEditor.01.00.ecschema.xml");

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();
    WString schemaPath = BeFileName::GetDirectoryName (ecSchemaPath);
    schemaContext->AddSchemaPath (schemaPath.c_str ());

    ECSchemaPtr schema;
    ECSchema::ReadFromXmlFile (schema, ecSchemaPath, *schemaContext);
    ASSERT_TRUE (schema.IsValid ());

    ASSERT_EQ (SUCCESS, ImportSchema (*schema, *dgnProj));

    ASSERT_TRUE (dgnProj->TableExists ("sdde_ArchStoreyWithElements"));
    ASSERT_TRUE (dgnProj->TableExists ("sdde_ArchWithHVACStorey"));
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
    // ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(dbPath, L"3dMetricGeneral.idgndb", L"DgnProject.DuplicateElementId.dgndb", __FILE__));
    
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad Hassan                         12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnProjectPackageTest, VerifyViewsForDgndbFilesConvertedDuringBuild)
    {
    std::vector<Utf8String> dgndbFiles;
    dgndbFiles.push_back ("79_Main.i.idgndb");
    dgndbFiles.push_back ("04_Plant.i.idgndb");
    dgndbFiles.push_back ("BGRSubset.i.idgndb");
    dgndbFiles.push_back ("fonts.dgn.i.idgndb");
    dgndbFiles.push_back ("2dMetricGeneral.idgndb");
    dgndbFiles.push_back ("3dMetricGeneral.idgndb");

    BeFileName dgndbFilesPath;
    BeTest::GetHost ().GetDocumentsRoot (dgndbFilesPath);
    dgndbFilesPath.AppendToPath (L"DgnDb");

    DbResult status;
    DgnDbPtr dgnProj;

    for (auto dgndbFileName : dgndbFiles)
        {
        BeFileName filePath = dgndbFilesPath;
        filePath.AppendToPath ((BeFileName)dgndbFileName);
        
        dgnProj = DgnDb::OpenDgnDb (&status, filePath, DgnDb::OpenParams (Db::OpenMode::ReadWrite));
        ASSERT_EQ (DbResult::BE_SQLITE_OK, status) << status;

        Statement statement;
        ASSERT_EQ (DbResult::BE_SQLITE_OK, statement.Prepare (*dgnProj, "select '[' || name || ']'  from sqlite_master where type = 'view' and instr (name,'.') and instr(sql, '--### ECCLASS VIEW')"));
        while (statement.Step () == BE_SQLITE_ROW)
            {
            //printf ("\n Schema : %s ,     View Name : %s   \n", statement.GetValueText (0), statement.GetValueText (1));
            Statement stmt;
            Utf8String sql;
            sql.Sprintf ("SELECT * FROM %s", statement.GetValueText (0));
            ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (*dgnProj, sql.c_str ())) << "Prepare failed : " << sql.c_str () << " in DgnDb : " << dgndbFileName.c_str ();
            }
        statement.Finalize ();

        dgnProj->CloseDb ();
        }
    }

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      01/15
+===============+===============+===============+===============+===============+======*/
struct QueryElementIdGraphiteURI : ::testing::Test
{
    BETEST_DECLARE_TC_SETUP
    BETEST_DECLARE_TC_TEARDOWN

    ScopedDgnHost m_host;

    static DgnDbTestUtils::SeedDbInfo s_seedFileInfo;

    QueryElementIdGraphiteURI()
        {
        // Must register my domain whenever I initialize a host
        DgnPlatformTestDomain::Register();
        }
};

DgnDbTestUtils::SeedDbInfo QueryElementIdGraphiteURI::s_seedFileInfo;

//---------------------------------------------------------------------------------------
// Do one-time setup for all tests in this group
// In this case, I just request the (root) seed file that my tests will use and make a note of it.
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
BETEST_TC_SETUP(QueryElementIdGraphiteURI) 
    {
    ScopedDgnHost tempHost;
    QueryElementIdGraphiteURI::s_seedFileInfo = DgnDbTestUtils::GetSeedDb(DgnDbTestUtils::SeedDbId::OneSpatialModel, DgnDbTestUtils::SeedDbOptions(true, true));
    }

//---------------------------------------------------------------------------------------
// Clean up what I did in my one-time setup
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
BETEST_TC_TEARDOWN(QueryElementIdGraphiteURI)
    {
    // Note: leave your subdirectory in place. Don't remove it. That allows the 
    // base class to detect and throw an error if two groups try to use a directory of the same name.
    // Don't worry about stale data. The test runner will clean out everything at the start of the program.
    // You can empty the directory, if you want to save space.
    //DgnDbTestUtils::EmptySubDirectory(GROUP_SUBDIR);
    }

TEST_F(QueryElementIdGraphiteURI, Test1)
    {
    // Note: We know that our group's TC_SETUP function has already created the group seed file. We can just ask for it.
    DgnDbPtr db = DgnDbTestUtils::OpenSeedDbCopy(s_seedFileInfo.fileName, L"Test1");
    ASSERT_TRUE(db.IsValid());

    DgnModelId mid = db->Models().QueryModelId(s_seedFileInfo.modelCode);
    DgnCategoryId catId = DgnCategory::QueryCategoryId(s_seedFileInfo.categoryName, *db);

    DgnElementCPtr el;
    if (true)
        {
        TestElementPtr testel = TestElement::Create(*db, mid, catId, "E1");
        testel->SetTestElementProperty("foo");
        el = testel->Insert();
        ASSERT_TRUE(el.IsValid());

        db->SaveChanges();
        }

    Utf8CP uri = "/DgnElements?ECClass=DgnPlatformTest:TestElement&TestElementProperty=foo";
    auto eid = db->Elements().QueryElementIdGraphiteURI(uri);
    ASSERT_TRUE(eid == el->GetElementId());

    Utf8CP baduri = "/DgnElements?ECClass=DgnPlatformTest:TestElement&TestElementProperty=bar";
    auto badeid = db->Elements().QueryElementIdGraphiteURI(baduri);
    ASSERT_TRUE(!badeid.IsValid());
    }

struct ImportTests : DgnDbTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ImportTests, simpleSchemaImport)
    {
    Utf8CP testSchemaXml = "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference name = 'dgn' version = '02.00' prefix = 'dgn' />"
        "  <ECSchemaReference name = 'ECDbMap' version = '01.01' prefix = 'ecdbmap' />"
        "  <ECClass typeName='Element1' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>dgn:PhysicalElement</BaseClass>"
        "    <ECProperty propertyName='Prop1_1' typeName='string' />"
        "    <ECProperty propertyName='Prop1_2' typeName='long' />"
        "    <ECProperty propertyName='Prop1_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element2' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element1</BaseClass>"
        "    <ECProperty propertyName='Prop2_1' typeName='string' />"
        "    <ECProperty propertyName='Prop2_2' typeName='long' />"
        "    <ECProperty propertyName='Prop2_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element3' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element2</BaseClass>"
        "    <ECProperty propertyName='Prop3_1' typeName='string' />"
        "    <ECProperty propertyName='Prop3_2' typeName='long' />"
        "    <ECProperty propertyName='Prop3_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element4' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element3</BaseClass>"
        "    <ECProperty propertyName='Prop4_1' typeName='string' />"
        "    <ECProperty propertyName='Prop4_2' typeName='long' />"
        "    <ECProperty propertyName='Prop4_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element4b' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element3</BaseClass>"
        "    <ECProperty propertyName='Prop4b_1' typeName='string' />"
        "    <ECProperty propertyName='Prop4b_2' typeName='long' />"
        "    <ECProperty propertyName='Prop4b_3' typeName='double' />"
        "    <ECProperty propertyName='Prop4b_4' typeName='point3d' />"
        "  </ECClass>"
        "</ECSchema>";

    SetupProject(L"3dMetricGeneral.idgndb", L"New3dMetricGeneralDb.idgndb", BeSQLite::Db::OpenMode::ReadWrite);
    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    m_db->SaveChanges();

    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaContext->AddSchemaPath(searchDir.GetName());

    ECN::ECSchemaPtr schema = nullptr;
    ASSERT_EQ(ECN::SchemaReadStatus::Success, ECN::ECSchema::ReadFromXmlString(schema, testSchemaXml, *schemaContext));
    ASSERT_TRUE(schema != nullptr);

    schemaContext->AddSchema(*schema);
    ASSERT_EQ(DgnDbStatus::Success, DgnBaseDomain::GetDomain().ImportSchema(*m_db, schemaContext->GetCache()));
    ASSERT_TRUE(m_db->IsDbOpen());
    }
