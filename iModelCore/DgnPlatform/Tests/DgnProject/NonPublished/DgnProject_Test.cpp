/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnProject_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/DgnIModel.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>

USING_NAMESPACE_BENTLEY_SQLITE 

//=======================================================================================
// @bsiclass                                                    Majd.Uddin   04/12
//=======================================================================================
struct TestModelProperties
    {
    public:
        DgnModelId      tmId;
        WString         tmName;
        WString         tmDescription;
        bool            tmIs3d;
        DgnModelType    tmModelType;

        void SetTestModelProperties (WString Name, WString Desc, bool is3D, DgnModelType modType)
            {
            tmName = Name;
            tmDescription = Desc;
            tmIs3d = is3D;
            tmModelType = modType;
            };
        void IsEqual (TestModelProperties Model)
            {
            EXPECT_STREQ (tmName.c_str(), Model.tmName.c_str()) << "Names don't match";
            EXPECT_STREQ (tmDescription.c_str(), Model.tmDescription.c_str()) << "Descriptions don't match";
            EXPECT_TRUE (tmIs3d == Model.tmIs3d) << "3dness doesn't match";
            EXPECT_TRUE (tmModelType == Model.tmModelType) << "Model Types don't match";
            };
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/11
//=======================================================================================
struct StepTimer
{
    WCharCP m_msg;
    uint64_t m_startTime;

    static uint64_t GetCurrentTime () {return BeTimeUtilities::QueryMillisecondsCounter();}

    StepTimer (WCharCP msg) {m_msg=msg; m_startTime=GetCurrentTime();}
    ~StepTimer() {wprintf (L"%ls, %lf Seconds\n", m_msg, (GetCurrentTime()-m_startTime)/1000.);}
};

static void openProject (DgnDbPtr& project, BeFileName const& projectFileName, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OPEN_Readonly)
    {
    DbResult result;
    project = DgnDb::OpenDgnDb (&result, projectFileName, DgnDb::OpenParams(mode));

    ASSERT_TRUE( project != NULL);
    ASSERT_TRUE( result == BE_SQLITE_OK );

    Utf8String projectFileNameUtf8 (projectFileName.GetName());
    ASSERT_TRUE( projectFileNameUtf8 == Utf8String(project->GetDbFileName  ()));
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
    PropertySpec test1 ("test1", "testapp", PropertySpec::TXN_MODE_Setting);

    BeFileName projectName;

    if (true)
        {
        DgnDbTestDgnManager tdm (L"2dMetricGeneral.idgndb", __FILE__, Db::OPEN_ReadWrite);
        DgnDbP newProject = tdm.GetDgnProjectP();
        ASSERT_TRUE( newProject != NULL );
    
        projectName.SetNameUtf8 (newProject->GetDbFileName());

        // establish inititial value
        newProject->SavePropertyString (test1, val1);

        Utf8String val;
        newProject->QueryProperty (val, test1);         // make sure we can read it back
        ASSERT_TRUE (0 == strcmp (val.c_str(), val1));

        newProject->SavePropertyString (test1, val2);   // change the value
        newProject->QueryProperty (val, test1);         // and verify we get the new value back
        ASSERT_TRUE (0 == strcmp (val.c_str(), val2));

        newProject->SaveSettings();                     // save to file
        newProject->QueryProperty (val, test1);         // read it back
        ASSERT_TRUE (0 == strcmp (val.c_str(), val2));  // make sure it is still unchanged

        newProject->SavePropertyString (test1, val3);   // change it to a new value again
        newProject->QueryProperty (val, test1);
        ASSERT_TRUE (0 == strcmp (val.c_str(), val3));  // make sure we have new value in temp table.
        } // NOW - close the file. The setting should be saved persistently in its second state.

    // reopen the project
    DgnDbPtr sameProjectPtr;
    openProject (sameProjectPtr, projectName, BeSQLite::Db::OPEN_ReadWrite);
    ASSERT_TRUE( sameProjectPtr.IsValid());

    Utf8String val;
    sameProjectPtr->QueryProperty (val, test1);
    ASSERT_TRUE (0 == strcmp (val.c_str(), val2));      // make sure we get "value 2"

    sameProjectPtr->DeleteProperty(test1);              // delete the setting
    BeSQLite::DbResult rc = sameProjectPtr->QueryProperty (val, test1);    // verify we can't read it any more.
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

    DgnDbTestDgnManager tdm (L"2dMetricGeneral.idgndb", __FILE__, Db::OPEN_Readonly);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    // Check that std properties are in the be_Props table. We can only check the value of a few using this API.
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("DbGuid",            "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("BeSQLiteBuild",     "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("CreationDate",      "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("Description",       "dgn_Proj"      )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("Units",             "dgn_Proj"      )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("LastEditor",        "dgn_Proj"      )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("DefaultView",       "dgn_View"      )) );
    
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("SchemaVersion",     "be_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("SchemaVersion",     "ec_Db"         )) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("SchemaVersion",     "dgn_Proj"      )) );

    //  Use the model API to access model properties and check their values
    DgnModelP defaultModel = project->Models().GetModel(project->Models().QueryFirstModelId());

    //  Use ModelInfo as an alt. way to get at some of the same property data
    DgnModel::Properties const& minfo = defaultModel->GetProperties();
    ASSERT_TRUE( minfo.GetMasterUnit().GetBase() == UnitBase::Meter );
    ASSERT_TRUE( minfo.GetSubUnit().GetBase() == UnitBase::Meter );
    }

/*---------------------------------------------------------------------------------**//**
* Schema Version can be accessed and it is correct
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnDb, ProjectSchemaVersions)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb", __FILE__, Db::OPEN_ReadWrite, TestDgnManager::DGNINITIALIZEMODE_None);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);

    // Get Schema version details
    DgnVersion schemaVer = project->GetSchemaVersion();
    ASSERT_EQ (DGNDB_CURRENT_VERSION_Major, schemaVer.GetMajor()) << "The Schema Major Version is: " << schemaVer.GetMajor();
    ASSERT_EQ (DGNDB_CURRENT_VERSION_Minor, schemaVer.GetMinor()) << "The Schema Minor Version is: " << schemaVer.GetMinor();
    ASSERT_EQ (DGNDB_CURRENT_VERSION_Sub1, schemaVer.GetSub1()) << "The Schema Sub1 Version is: " << schemaVer.GetSub1();
    ASSERT_EQ (DGNDB_CURRENT_VERSION_Sub2, schemaVer.GetSub2()) << "The Schema Sub2 Version is: " << schemaVer.GetSub2();
    }

/*---------------------------------------------------------------------------------**//**
* Getting the list of Dgn Models in a project and see if they work
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnDb, WorkWithDgnModelTable)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"ElementsSymbologyByLevel.idgndb", __FILE__, Db::OPEN_Readonly);
    DgnDbP project = tdm.GetDgnProjectP();
    //*** Issue while using demo.dgn on Merging models. using another file instead that has 2 models.
    ASSERT_TRUE( project != NULL);

    //Iterating through the models
    DgnModels& modelTable = project->Models();
    DgnModels::Iterator iter = modelTable.MakeIterator();
    ASSERT_EQ (1, iter.QueryCount());

    //Set up testmodel properties as we know what the models in this file contain
    TestModelProperties models[4], testModel;
    models[0].SetTestModelProperties (L"Default", L"Master Model", true, DgnModelType::Physical);

    //Iterate through the model and verify it's contents. TODO: Add more checks
    int i = 0;
    for (DgnModels::Iterator::Entry const& entry : iter)
        {
        ASSERT_TRUE (entry.GetModelId().IsValid()) << "Model Id is not Valid";
        WString entryNameW (entry.GetName(), true);               // string conversion
        WString entryDescriptionW (entry.GetDescription(), true); // string conversion
        testModel.SetTestModelProperties (entryNameW.c_str(), entryDescriptionW.c_str(), entry.Is3d(), entry.GetModelType());
        testModel.IsEqual (models[i]);
        i++;
        }
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct CategoryTestData
    {
    DgnCategories& m_categories;
    DgnCategoryId m_category1;
    DgnCategoryId m_category2;
    CategoryTestData (DgnCategories& categories, DgnCategoryId category1, DgnCategoryId category2) : m_categories(categories), m_category1(category1), m_category2(category2) {}
    };

static DgnSubCategoryId facetId1, facetId2;

/*---------------------------------------------------------------------------------**//**
* Load a Model directly
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnDb, LoadModelThroughProject)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"ElementsSymbologyByLevel.idgndb", __FILE__, Db::OPEN_Readonly, TestDgnManager::DGNINITIALIZEMODE_None);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);

    //Load a Model directly. First get the ModelId
    DgnModels& modelTable = project->Models();
    DgnModelId modelId = modelTable.QueryModelId("Default");
    ASSERT_TRUE (modelId.IsValid());

    DgnModelP model = project->Models().GetModel (modelId);
    ASSERT_TRUE (NULL != model);

    //Just one more check to see that we got the right model
    EXPECT_STREQ ("Default", model->GetModelName());
    }

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
    BeFileName::BeDeleteFile (DgnDbTestDgnManager::GetOutputFilePath (L"dup.idgndb"));

    //Create and Verify that project was created
    project = DgnDb::CreateDgnDb (&status, DgnDbTestDgnManager::GetOutputFilePath (L"dup.idgndb"), params);
    ASSERT_TRUE (project != NULL);
    ASSERT_EQ (BE_SQLITE_OK, status) << "Status returned is:" << status;

    //Create another project with same name. It should fail
    project2 = DgnDb::CreateDgnDb (&status2, DgnDbTestDgnManager::GetOutputFilePath (L"dup.idgndb"), params);
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
    TestDataManager::FindTestData (fullFileName, L"ElementsSymbologyByLevel.idgndb", DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    
    BeFileName testFile = DgnDbTestDgnManager::GetOutputFilePath (L"ElementsSymbologyByLevel.idgndb");
    BeFileName::BeCopyFile (fullFileName, testFile);

    DbResult status1;
    DgnDbPtr dgnProj1;
    dgnProj1 = DgnDb::OpenDgnDb (&status1, testFile, DgnDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Exclusive));
    EXPECT_EQ (BE_SQLITE_OK, status1) << status1;
    ASSERT_TRUE (dgnProj1 != NULL);

    DbResult status2;
    DgnDbPtr dgnProj2;
    dgnProj2 = DgnDb::OpenDgnDb (&status2, testFile, DgnDb::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Exclusive));
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
    StatusInt testDataFound = TestDataManager::FindTestData (path, L"ECSqlTest.01.00.ecschema.xml", BeFileName (L"DgnDb\\ECDb\\Schemas"));
    ASSERT_TRUE (SUCCESS == testDataFound);

    DbResult status;
    dgnProj = DgnDb::OpenDgnDb (&status, path, DgnDb::OpenParams(Db::OPEN_Readonly));
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
    BeTest::GetHost().GetOutputRoot (dgndbFileName);
    dgndbFileName.AppendToPath (L"MyFile.idgndb");

     if (BeFileName::DoesPathExist (dgndbFileName))
        BeFileName::BeDeleteFile (dgndbFileName);
    DbResult status;
    CreateDgnDbParams Obj;
    dgnProj = DgnDb::CreateDgnDb(&status, BeFileName (dgndbFileName.GetNameUtf8().c_str()),Obj);
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
    BeTest::GetHost().GetOutputRoot (dgndbFileName);
    dgndbFileName.AppendToPath (L"MyTextDGNDBFILE.txt");
    if (BeFileName::DoesPathExist (dgndbFileName))
        BeFileName::BeDeleteFile (dgndbFileName);

    DbResult status;
    CreateDgnDbParams Obj;
    dgnProj = DgnDb::CreateDgnDb(&status, BeFileName (dgndbFileName.GetNameUtf8().c_str()),Obj);
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
    BeTest::GetHost().GetOutputRoot (dgndbFileNotExist);
    dgndbFileNotExist.AppendToPath (L"MyFileNotExist.idgndb");

    dgnProj = DgnDb::OpenDgnDb (&status, BeFileName (dgndbFileNotExist.GetNameUtf8().c_str()), DgnDb::OpenParams(Db::OPEN_Readonly));
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
    BeTest::GetHost().GetOutputRoot (dgndbFileName);
    dgndbFileName.AppendToPath (testFileName);

    BeFileName::BeCopyFile (sourceFile, dgndbFileName, false);

    DbResult status;
    DgnDbPtr dgnProj = DgnDb::OpenDgnDb (&status, dgndbFileName, DgnDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Exclusive));
    EXPECT_EQ (BE_SQLITE_OK, status) << status;
    ASSERT_TRUE( dgnProj != NULL);

    // once a Db is opened for ReadWrite with exclusive access, it can't be opened, even for read.
    DgnDbPtr dgnProj1 = DgnDb::OpenDgnDb (&status, dgndbFileName, DgnDb::OpenParams(Db::OPEN_Readonly));
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
    TestDataManager::FindTestData (fullFileName, L"GeoCoordinateSystem.i.idgndb", DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    //Open project
    DgnDbPtr dgnProj;
    openProject (dgnProj, fullFileName, BeSQLite::Db::OPEN_Readonly);
    double azmth = dgnProj->Units().GetAzimuth();
    double azmthExpected = 178.2912;
    double eps = 0.0001;
    EXPECT_TRUE(fabs(azmthExpected - azmth) < eps )<<"Expected diffrent azimuth ";
    GeoPoint gorigin;
    dgnProj->Units().LatLongFromUors(gorigin, DPoint3d::FromZero());
    double const latitudeExpected = 42.3413;
    EXPECT_TRUE(fabs(latitudeExpected - gorigin.latitude) < eps)<<"Expected diffrent latitude ";
    double const longitudeExpected = -71.0806;
    EXPECT_TRUE(fabs (longitudeExpected - gorigin.longitude) < eps)<<"Expected diffrent longitude ";
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
            project->QueryProperty (properties.version, DgnProjectProperty::SchemaVersion());
            project->QueryProperty (properties.name, DgnProjectProperty::Name());
            project->QueryProperty (properties.description, DgnProjectProperty::Description());
            project->QueryProperty (properties.client, DgnProjectProperty::Client());
            project->QueryProperty (properties.lastEditor, DgnProjectProperty::LastEditor());
            project->QueryProperty (properties.creationDate, Properties::CreationDate());
            }
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                            Julija.Suboc                08/13
        +---------------+---------------+---------------+---------------+---------------+------*/
        void getPackageProperties(BeSQLite::Db& db, PropertiesInTable& properties, uint64_t embeddedFileId)
            {
            db.QueryProperty (properties.version, DgnEmbeddedProjectProperty::SchemaVersion(), embeddedFileId);
            db.QueryProperty (properties.name, DgnEmbeddedProjectProperty::Name(), embeddedFileId);
            db.QueryProperty (properties.description, DgnEmbeddedProjectProperty::Description(), embeddedFileId);
            db.QueryProperty (properties.client, DgnEmbeddedProjectProperty::Client(), embeddedFileId);
            db.QueryProperty (properties.lastEditor, DgnEmbeddedProjectProperty::LastEditor(), embeddedFileId);
            db.QueryProperty (properties.creationDate, DgnEmbeddedProjectProperty::CreationDate(), embeddedFileId);
            }
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                            Julija.Suboc                08/13
        +---------------+---------------+---------------+---------------+---------------+------*/
        void compareProperties(PropertiesInTable& prop, PropertiesInTable& propV)
            {
            EXPECT_TRUE(prop.version.CompareTo(prop.version)==0)<<"Version does not match";
            EXPECT_TRUE(prop.name.CompareTo(prop.name)==0)<<"Name does not match";
            EXPECT_TRUE(prop.description.CompareTo(prop.description)==0)<<"Description does not match";
            EXPECT_TRUE(prop.client.CompareTo(prop.client)==0)<<"Client does not match";
            EXPECT_TRUE(prop.lastEditor.CompareTo(prop.lastEditor)==0)<<"Last editor does not match";
            EXPECT_TRUE(prop.creationDate.CompareTo(prop.creationDate)==0)<<"Creation date does not match";
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
                DgnModelP model = project->Models().GetModel(entry.GetModelId());
                model->FillModel();
                properties.elmCount += model->CountElements();
                properties.modelCount++;
                }
            properties.viewCount = project->Views().MakeIterator().QueryCount();
            properties.categoryCount = project->Categories().MakeIterator().QueryCount();
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
    TestDataManager::FindTestData (fullFileName, L"ElementsSymbologyByLevel.idgndb", DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    BeFileName testFile = DgnDbTestDgnManager::GetOutputFilePath (L"ElementsSymbologyByLevel.idgndb");
    BeFileName::BeCopyFile (fullFileName, testFile);
    //Collect properties for later verification
    DbResult status;
    DgnDbPtr dgnProj;
    openProject (dgnProj, testFile, BeSQLite::Db::OPEN_ReadWrite);
    PropertiesInTable propertiesInTable;
    getPropertiesInTable(dgnProj, propertiesInTable);
    dgnProj->CloseDb();
    //Create package and open it for verification
    BeFileName packageFile = DgnDbTestDgnManager::GetOutputFilePath (L"package.db");
    CreateIModelParams createParams;
    createParams.SetOverwriteExisting(true);
    status =  DgnIModel::Create(packageFile, testFile, createParams);
    ASSERT_EQ (DGNFILE_STATUS_Success, status); 
    BeSQLite::Db        db;
    Db::OpenParams      openParams(Db::OPEN_Readonly);
    DbResult dbResult = db.OpenBeSQLiteDb(packageFile, openParams);
    ASSERT_TRUE(dbResult == BE_SQLITE_OK)<<"Failed to open database with compressed file";
    //Check embedded files table and get file id
    DbEmbeddedFileTable& embeddedFiles = db.EmbeddedFiles();
    ASSERT_EQ(1, embeddedFiles.MakeIterator().QueryCount())<<"There should be only one embeded file";
    BeRepositoryBasedId fileId = embeddedFiles.QueryFile("ElementsSymbologyByLevel.idgndb");
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
    TestDataManager::FindTestData (fullFileName, testDatabase, DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    BeFileName testFile = DgnDbTestDgnManager::GetOutputFilePath (L"ElementsSymbologyByLevel.idgndb");
    BeFileName::BeCopyFile (fullFileName, testFile);
    //Get some information which can be used later to verify if file was extracted correctly
    DbResult status;
    DgnDbPtr dgnProj;
    openProject (dgnProj, testFile, BeSQLite::Db::OPEN_ReadWrite);
    //Get properties for later verification
    ProjectProperties projectProp;
    getProjectProperties(dgnProj, projectProp);
    PropertiesInTable properties;
    getPropertiesInTable(dgnProj, properties);
    dgnProj->CloseDb();
    //Create package
    BeFileName packageFile = DgnDbTestDgnManager::GetOutputFilePath (L"package.db");
    CreateIModelParams createParams;
    createParams.SetOverwriteExisting(true);
    status =  DgnIModel::Create(packageFile, testFile, createParams);
    EXPECT_EQ (DGNFILE_STATUS_Success, status);
    //Extract package
    DbResult dbResult;
    //Prepare directory where file will be extracted
    BeFileName extractedFileDir;
    extractedFileDir.AppendToPath(DgnDbTestDgnManager::GetOutputFilePath (L"extractedFile"));
    if(!BeFileName::IsDirectory(extractedFileDir.GetName()))
        BeFileName::CreateNewDirectory(extractedFileDir.GetName());
    Utf8String fileName;
    BeStringUtilities::WCharToUtf8(fileName, testDatabase);
    auto filestat = DgnIModel::Extract(dbResult, extractedFileDir.GetNameUtf8().c_str(), fileName.c_str(), packageFile, true);
    EXPECT_EQ(DGNFILE_STATUS_Success, filestat);
    EXPECT_TRUE(BE_SQLITE_OK == dbResult);
    //Open project that was extracted from package  
    DgnDbPtr dgnProjV;
    BeFileName extractedFile(extractedFileDir.GetNameUtf8());
    extractedFile.AppendToPath(BeFileName::GetFileNameAndExtension(testFile.GetName()).c_str());
    openProject (dgnProjV, extractedFile, BeSQLite::Db::OPEN_ReadWrite);
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
    TestDataManager::FindTestData (fullFileName, fileName, DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    BeFileName testFile = DgnDbTestDgnManager::GetOutputFilePath (fileName);
    ASSERT_TRUE(BeFileNameStatus::Success ==BeFileName::BeCopyFile (fullFileName, testFile))<<"Failed to copy file";
    //Open project
    DbResult status;
    DgnDbPtr dgnProj;
    openProject (dgnProj, testFile, BeSQLite::Db::OPEN_ReadWrite);
    //Get properties for later verification
    ProjectProperties projectProp;
    getProjectProperties(dgnProj, projectProp);
    PropertiesInTable properties;
    getPropertiesInTable(dgnProj, properties);
    dgnProj->CloseDb();
    
    //Create package
    BeFileName packageFile = DgnDbTestDgnManager::GetOutputFilePath (L"package.db");
    CreateIModelParams createParams;
    createParams.SetOverwriteExisting(true);
    status =  DgnIModel::Create(packageFile, testFile, createParams);
    EXPECT_EQ (DGNFILE_STATUS_Success, status);
    //Extract file from package
    DbResult dbResult;
    BeFileName extractedFile = DgnDbTestDgnManager::GetOutputFilePath (L"extractedUsingDefaults.idgndb");
    auto fileStatus = DgnIModel::ExtractUsingDefaults(dbResult, extractedFile, packageFile, true);
    EXPECT_EQ(DGNFILE_STATUS_Success, fileStatus);
    EXPECT_TRUE(BE_SQLITE_OK == dbResult);
   
    //Open file for verification
    DgnDbPtr dgnProjV;
    openProject (dgnProjV, extractedFile, BeSQLite::Db::OPEN_ReadWrite);
   //Verify that properties did not change
    ProjectProperties projectPropV;
    getProjectProperties(dgnProjV, projectPropV);
    compareProjectProperties(projectProp, projectPropV);
    PropertiesInTable propertiesV;
    getPropertiesInTable(dgnProjV, propertiesV);
    compareProperties(properties, propertiesV);
    }
