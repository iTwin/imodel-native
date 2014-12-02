/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnProject_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt createLineElement (EditElementHandle& eh, DgnModelR model, size_t nLinesCreated)
    {
    DSegment3d      segment;
    segment.Init (0,0,0, 0,0,0);
    segment.point[1].x += 10000;
    segment.point[0].y = segment.point[1].y = (double)nLinesCreated * 10000;
    ExtendedElementHandler::InitializeElement (eh, NULL, model, model.Is3d());
    return SUCCESS;
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/11
//=======================================================================================
struct StepTimer
{
    WCharCP m_msg;
    UInt64 m_startTime;

    static UInt64 GetCurrentTime () {return BeTimeUtilities::QueryMillisecondsCounter();}

    StepTimer (WCharCP msg) {m_msg=msg; m_startTime=GetCurrentTime();}
    ~StepTimer() {wprintf (L"%ls, %lf Seconds\n", m_msg, (GetCurrentTime()-m_startTime)/1000.);}
};

static void openProject (DgnProjectPtr& project, BeFileName const& projectFileName, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OPEN_Readonly)
    {
    DgnFileStatus result;
    project = DgnProject::OpenProject (&result, projectFileName, DgnProject::OpenParams(mode));

    ASSERT_TRUE( project != NULL);
    ASSERT_TRUE( result == DGNFILE_STATUS_Success);

    Utf8String projectFileNameUtf8 (projectFileName.GetName());
    ASSERT_TRUE( projectFileNameUtf8 == Utf8String(project->GetDbFileName  ()));
    }

#if defined (NEEDS_WORK_DGNITEM)
static void inspectDgnFile (DgnProjectP project, bool isReadOnly, bool expectFilledModels)
    {
    ASSERT_TRUE( project->IsReadonly() == isReadOnly );
    ASSERT_TRUE( project->IsDbOpen() );

    bool foundDefault = false;
    size_t modelCount=0;
    for (auto const& entry : project->Models().MakeIterator ())
        {
        foundDefault |= project->Models().GetFirstModelId() == entry.GetModelId();

        DgnModelP model = project->Models().GetModelById (entry.GetModelId());
        ASSERT_TRUE( model != NULL );
        ASSERT_TRUE( expectFilledModels == model->IsFilled());
        modelCount++;
        }
    ASSERT_TRUE( modelCount == 1 ) << "The model count should have been 1 but it is: " << modelCount;
    ASSERT_TRUE( foundDefault );

    StatusInt mresult;
    DgnModelP defaultModel = project->Models().GetAndFillModelById (&mresult, project->Models().GetFirstModelId());
    ASSERT_TRUE( defaultModel != NULL );
    ASSERT_TRUE( mresult == SUCCESS );
    ASSERT_TRUE( defaultModel == project->Models().FindModelById (defaultModel->GetModelId()) );
    ASSERT_TRUE( expectFilledModels == defaultModel->IsFilled ());
    if (!expectFilledModels)
        ASSERT_TRUE( defaultModel->FillSections() == SUCCESS );

    size_t ecount=0;
    FOR_EACH (PersistentElementRefP ref, defaultModel->GetElementsCollection ())
        {
        ElementHandle eh (ref);

        // *** NEEDS WORK

        ++ecount;

        // *** NEEDS WORK
        }
    }


static size_t findElementById (ElementRefP* refH, DgnModelR model, ElementId eid)
    {
    size_t found = 0;
    FOR_EACH (PersistentElementRefP ref, model.GetElementsCollection ())
        {
        if (ref->GetElementId() == eid)
            {
            if (refH)
                *refH = ref;
            ++found;
            }
        }
    return found;
    }
#endif

// This test checks that we can use DgnFile/DgnModel API to modify elements in a dgndb
TEST(DgnProject,AddDeleteLine)
    {
    ScopedDgnHost autoDgnHost;
    BeFileName projectName;

    ElementId eid;

    if (true)
        {
        DgnDbTestDgnManager tdm (L"2dMetricGeneral.idgndb", __FILE__, OPENMODE_READWRITE);
        DgnProjectP project = tdm.GetDgnProjectP();
        ASSERT_TRUE( project != NULL);
        
        projectName.SetNameUtf8 (project->GetDbFileName());

        DgnModelP defaultModel = project->Models().GetAndFillModelById (NULL, project->Models().GetFirstModelId());
        defaultModel->FillModel();

        size_t nLinesCreated = 0;
        EditElementHandle eh;
        createLineElement (eh, *defaultModel, nLinesCreated);

        ASSERT_TRUE( eh.AddToModel () == SUCCESS );

        eid = eh.GetElementId();

#if defined (NEEDS_WORK_DGNITEM)
        ASSERT_TRUE( findElementById (NULL, *defaultModel, eid) == 1 );
#endif
        }

    // re-open and check that element is there.
    if (true)
        {
        DgnProjectPtr project;
        openProject (project, projectName, BeSQLite::Db::OPEN_ReadWrite);
        ASSERT_TRUE( project != NULL);
#if defined (NEEDS_WORK_DGNITEM)
        inspectDgnFile (project.get(),/*isReadOnly*/false, false);
#endif

        DgnModelP defaultModel = project->Models().GetAndFillModelById (NULL, project->Models().GetFirstModelId());
        ASSERT_TRUE( defaultModel != NULL );
        ASSERT_TRUE( defaultModel->FillModel() == SUCCESS );

#if defined (NEEDS_WORK_DGNITEM)
        ElementRefP ref = NULL;
        ASSERT_TRUE( findElementById (&ref, *defaultModel, eid) == 1 );

        // delete the line
        EditElementHandle eh (ref);
        eh.DeleteFromModel ();

        ASSERT_TRUE( findElementById (NULL, *defaultModel, eid) == 0 );
#endif
        }

    // re-open and check that element is still gone.
    if (true)
        {
        DgnProjectPtr project;
        openProject (project, projectName, BeSQLite::Db::OPEN_Readonly);
        ASSERT_TRUE( project != NULL);
#if defined (NEEDS_WORK_DGNITEM)
        inspectDgnFile (project.get(),/*isReadOnly*/true, false);
#endif

        DgnModelP defaultModel = project->Models().GetAndFillModelById (NULL, project->Models().GetFirstModelId());
        ASSERT_TRUE( defaultModel != NULL );
        ASSERT_TRUE( defaultModel->FillModel() == SUCCESS );

        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DgnProject, Settings)
    {
    ScopedDgnHost autoDgnHost;

    Utf8CP val1 = "value 1";
    Utf8CP val2 = "value 2";
    Utf8CP val3 = "value 3";
    PropertySpec test1 ("test1", "testapp", PropertySpec::TXN_MODE_Setting);

    BeFileName projectName;

    if (true)
        {
        DgnDbTestDgnManager tdm (L"2dMetricGeneral.idgndb", __FILE__, OPENMODE_READWRITE);
        DgnProjectP newProject = tdm.GetDgnProjectP();
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
    DgnProjectPtr sameProjectPtr;
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
TEST (DgnProject, CheckStandardProperties)
    {
    ScopedDgnHost autoDgnHost;

    //DbResult rc;
    Utf8String val;

    DgnDbTestDgnManager tdm (L"2dMetricGeneral.idgndb", __FILE__, OPENMODE_READONLY);
    DgnProjectP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    // Check that std properties are in the be_Props table. We can only check the value of a few using this API.
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("DbGuid",            "be_Db"         )    ) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("ProjectGuid",       "be_Db"         )    ) );

    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("SchemaVersion",     "dgn_Proj"      )    ) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("LastEditor",        "dgn_Proj"      )    ) );
    ASSERT_TRUE( val=="SampleIDgnToIDgnDbPublisher" ) << "BeTest overrides _SupplyProductName";
    ASSERT_TRUE( project->HasProperty (PropertySpec("ColorTable",        "dgn_Proj"      )    ) );

    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("ModelSettings",     "dgn_Model"     ),  0) );
    ASSERT_EQ( BE_SQLITE_ROW, project->QueryProperty (val, PropertySpec("ModelProps",        "dgn_Model"     ),  0) );

    //  Use the model API to access model properties and check their values
    DgnModelP defaultModel = project->Models().GetAndFillModelById (NULL, project->Models().GetFirstModelId());

    //  Use ModelInfo as an alt. way to get at some of the same property data
    ModelInfoCR minfo = defaultModel->GetModelInfo ();
    ASSERT_TRUE( minfo.GetMasterUnit().GetBase() == UnitBase::Meter );
    ASSERT_TRUE( minfo.GetSubUnit().GetBase() == UnitBase::Meter );
    }

/*---------------------------------------------------------------------------------**//**
* Schema Version can be accessed and it is correct
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, ProjectSchemaVersions)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb", __FILE__, OPENMODE_READWRITE, TestDgnManager::DGNINITIALIZEMODE_None);
    DgnProjectP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);
#if defined (NEEDS_WORK_DGNITEM)
    inspectDgnFile (project, /*isReadOnly*/false, false);
#endif

    // Get Schema version details
    DgnVersion schemaVer = project->GetSchemaVersion();
    ASSERT_EQ (PROJECT_CURRENT_VERSION_Major, schemaVer.GetMajor()) << "The Schema Major Version is: " << schemaVer.GetMajor();
    ASSERT_EQ (PROJECT_CURRENT_VERSION_Minor, schemaVer.GetMinor()) << "The Schema Minor Version is: " << schemaVer.GetMinor();
    ASSERT_EQ (PROJECT_CURRENT_VERSION_Sub1, schemaVer.GetSub1()) << "The Schema Sub1 Version is: " << schemaVer.GetSub1();
    ASSERT_EQ (PROJECT_CURRENT_VERSION_Sub2, schemaVer.GetSub2()) << "The Schema Sub2 Version is: " << schemaVer.GetSub2();
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* Getting the list of opened project. [[Linking Error on the use of DgnProject::GetOpenedProjects()
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, WorkWithProjectsList)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb", __FILE__, OPENMODE_READWRITE, TestDgnManager::DGNINITIALIZEMODE_None);
    DgnProjectP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);
    inspectDgnFile (project, /*isReadOnly*/false, false);

#if defined (WIP_NEEDS_WORK)
    // Open this project
    project = NULL;
    openProject (project, DgnDbTestDgnManager::GetOutputFilePath  (L"TestForList.idgndb"));
    ASSERT_TRUE( project != NULL);
#endif
    }
#endif

/*---------------------------------------------------------------------------------**//**
* Getting the list of Dgn Models in a project and see if they work
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, WorkWithDgnModelTable)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"ElementsSymbologyByLevel.idgndb", __FILE__, OPENMODE_READONLY);
    DgnProjectP project = tdm.GetDgnProjectP();
    //*** Issue while using demo.dgn on Merging models. using another file instead that has 2 models.
    ASSERT_TRUE( project != NULL);

    //Iterating through the models
    DgnModels& modelTable = project->Models();
    DgnModels::Iterator iter = modelTable.MakeIterator();
    ASSERT_EQ (3, iter.QueryCount()) << "The expected model count is 2 for ElementsSymbologyByLevel.dgn. Where as it is: " << iter.QueryCount();

    //Set up testmodel properties as we know what the models in this file contain
    TestModelProperties models[4], testModel;
    models[0].SetTestModelProperties (L"Default", L"Master Model", false, DgnModelType::Drawing);
    models[1].SetTestModelProperties (L"Model2d", L"", false, DgnModelType::Drawing);
    models[2].SetTestModelProperties (L"Default [master]", L"Master Model", false, DgnModelType::Drawing);
    models[3].SetTestModelProperties (L"Default [ref]", L"Master Model", false, DgnModelType::Drawing);

    //Iterate through the model and verify it's contents. TODO: Add more checks
    int i = 0;
    FOR_EACH (DgnModels::Iterator::Entry const& entry, iter)
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
struct LevelTestData
    {
    DgnLevels& m_levels;
    LevelId    m_level1;
    LevelId    m_level2;
    LevelTestData (DgnLevels& levels, LevelId level1, LevelId level2) : m_levels(levels), m_level1(level1), m_level2(level2) {}
    };

static DgnSubLevelId facetId1, facetId2;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, Stamps)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnProjectP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    Byte data[100];
    for (int i=0; i<100; ++i)
        data[i] = (Byte) i;

    DgnStamps::StampData data1(data, 10);
    DgnStamps::StampData data3(data+10, 11);

    DgnStamps::StampName name1("fred", "test");
    EXPECT_EQ(BE_SQLITE_OK, project->Stamps().InsertStamp(name1, data1));
    EXPECT_EQ(1, name1.GetId1().GetValue());

    DgnStamps::StampName name2("fred", "test", DgnStampId(0x11));
    DgnStamps::StampName name3("bob", "test", DgnStampId(0x10));
    EXPECT_EQ(BE_SQLITE_OK, project->Stamps().InsertStamp(name3, data3));

    DgnStamps::StampName name4("bob", ""); // illegal name
    EXPECT_NE(BE_SQLITE_OK, project->Stamps().InsertStamp(name4, data3));

    DgnStamps::StampName name5("fred", "test");
    EXPECT_EQ(BE_SQLITE_OK, project->Stamps().InsertStamp(name5, data1));
    EXPECT_EQ(2, name5.GetId1().GetValue());

    if (true)
        {
        auto stamp1Ptr = project->Stamps().FindStamp(name1);
        EXPECT_TRUE (stamp1Ptr.IsValid());
        EXPECT_EQ (stamp1Ptr->size(), data1.size());
        EXPECT_EQ (0, memcmp (stamp1Ptr->begin(), data1.begin(), data1.size()));

        auto stamp1Ptr2 = project->Stamps().FindStamp(name1);
        EXPECT_TRUE (stamp1Ptr.get() == stamp1Ptr2.get());
        EXPECT_EQ (3, stamp1Ptr->GetRefCount());
        }

    auto stamp2Ptr = project->Stamps().FindStamp(name2);
    EXPECT_FALSE (stamp2Ptr.IsValid());

    if (true)
        {
        auto stamp3Ptr = project->Stamps().FindStamp(name3);
        EXPECT_TRUE (stamp3Ptr.IsValid());
        EXPECT_EQ (stamp3Ptr->size(), data3.size());
        EXPECT_EQ (0, memcmp (stamp3Ptr->begin(), data3.begin(), data3.size()));
        }

    data3.clear();
    data3.insert (data3.begin(), data+20, data+50);
    EXPECT_EQ(BE_SQLITE_OK, project->Stamps().UpdateStamp(name3, data3));

    if (true)
        {
        auto stamp3Ptr = project->Stamps().FindStamp(name3);
        EXPECT_TRUE (stamp3Ptr.IsValid());
        EXPECT_EQ (stamp3Ptr->size(), data3.size());
        EXPECT_EQ (0, memcmp (stamp3Ptr->begin(), data3.begin(), data3.size()));
        }
    
    EXPECT_EQ(BE_SQLITE_OK, project->Stamps().DeleteStamp(name3));
    auto stamp3Ptr = project->Stamps().FindStamp(name3);
    EXPECT_FALSE (stamp3Ptr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* Load a Model directly
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, LoadModelThroughProject)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"ElementsSymbologyByLevel.idgndb", __FILE__, OPENMODE_READONLY, TestDgnManager::DGNINITIALIZEMODE_None);
    DgnProjectP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);

    //Load a Model directly. First get the ModelId
    DgnModels& modelTable = project->Models();
    DgnModelId modelId = modelTable.QueryModelId("Model2d");
    ASSERT_TRUE (modelId.IsValid());

    DgnModelP model = project->Models().GetModelById (modelId);
    ASSERT_TRUE (NULL != model);

    //Just one more check to see that we got the right model
    EXPECT_STREQ ("Model2d", model->GetModelName());
    }

/*---------------------------------------------------------------------------------**//**
* Creating a project with Duplicate name
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, ProjectWithDuplicateName)
    {
    ScopedDgnHost autoDgnHost;

    CreateProjectParams params;
    params.SetOverwriteExisting(false);
    DgnFileStatus status, status2;
    DgnProjectPtr project, project2;
    
    //Deleting the project file if it exists already
    BeFileName::BeDeleteFile (DgnDbTestDgnManager::GetOutputFilePath (L"dup.idgndb"));

    //Create and Verify that project was created
    project = DgnProject::CreateProject (&status, DgnDbTestDgnManager::GetOutputFilePath (L"dup.idgndb"), params);
    ASSERT_TRUE (project != NULL);
    ASSERT_EQ (DGNFILE_STATUS_Success, status) << "Status returned is:" << status;

    //Create another project with same name. It should fail
    project2 = DgnProject::CreateProject (&status2, DgnDbTestDgnManager::GetOutputFilePath (L"dup.idgndb"), params);
    EXPECT_FALSE (project2.IsValid()) << "Project with Duplicate name should not be created";
    EXPECT_EQ (DGNPROJECT_ERROR_CantCreateProjectFile, status2) << "Status returned for duplicate name is: " << status2;
    }

/*---------------------------------------------------------------------------------**//**
* Creating a project without importing Dgn and then try to access the DgnFile
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, ProjectWithoutDgn)
    {
    ScopedDgnHost autoDgnHost;

    CreateProjectParams params;
    DgnFileStatus status;
    DgnProjectPtr project;
    
    //Deleting the project file if it exists already
    BeFileName::BeDeleteFile (DgnDbTestDgnManager::GetOutputFilePath (L"EmptyProj.idgndb"));

    //Create and Verify that project was created
    project = DgnProject::CreateProject (&status, DgnDbTestDgnManager::GetOutputFilePath (L"EmptyProj.idgndb"), params);
    ASSERT_TRUE (project.IsValid());
    ASSERT_EQ (DGNFILE_STATUS_Success, status) << "Status returned is:" << status;

    //TO DO: Now get the DgnFile for this project
    //DgnProjectR dgnFile = project->GetDgnFile();
    //*** Exception here... Need to find a way on how to make sure that reference is not valid
    //ASSERT_TRUE (dgnFile.GetDgnFileId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* Try multiple read/write for project
* @bsimethod                                    Algirdas.Mikoliunas                02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, MultipleReadWrite)
    {
    ScopedDgnHost autoDgnHost;

    BeFileName fullFileName;
    TestDataManager::FindTestData (fullFileName, L"ElementsSymbologyByLevel.idgndb", DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    
    BeFileName testFile = DgnDbTestDgnManager::GetOutputFilePath (L"ElementsSymbologyByLevel.idgndb");
    BeFileName::BeCopyFile (fullFileName, testFile);

    DgnFileStatus status1;
    DgnProjectPtr dgnProj1;
    dgnProj1 = DgnProject::OpenProject (&status1, testFile, DgnProject::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Exclusive));
    EXPECT_EQ (DGNFILE_STATUS_Success, status1) << status1;
    ASSERT_TRUE (dgnProj1 != NULL);

    DgnFileStatus status2;
    DgnProjectPtr dgnProj2;
    dgnProj2 = DgnProject::OpenProject (&status2, testFile, DgnProject::OpenParams (Db::OPEN_ReadWrite, DefaultTxn_Exclusive));
    EXPECT_NE (DGNFILE_STATUS_Success, status2) << status2;
    ASSERT_TRUE (dgnProj2 == NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, InvalidFileFormat)
    {
    ScopedDgnHost           autoDgnHost;
    DgnProjectPtr      dgnProj;
    BeFileName path;
    StatusInt testDataFound = TestDataManager::FindTestData (path, L"OpenPlant.01.02.ecschema.xml", BeFileName (L"DgnDb\\ECDb\\Schemas"));
    ASSERT_TRUE (SUCCESS == testDataFound);

    DgnFileStatus status;
    dgnProj = DgnProject::OpenProject (&status, path, DgnProject::OpenParams(Db::OPEN_Readonly));
    EXPECT_EQ (DGNPROJECT_ERROR_CorruptDatabase, status) << status;
    ASSERT_TRUE( dgnProj == NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, CreateProject)
    {
    ScopedDgnHost           autoDgnHost;
    DgnProjectPtr      dgnProj;
    BeFileName dgndbFileName;
    BeTest::GetHost().GetOutputRoot (dgndbFileName);
    dgndbFileName.AppendToPath (L"MyFile.idgndb");

     if (BeFileName::DoesPathExist (dgndbFileName))
        BeFileName::BeDeleteFile (dgndbFileName);
    DgnFileStatus status;
    CreateProjectParams Obj;
    dgnProj = DgnProject::CreateProject(&status, BeFileName (dgndbFileName.GetNameUtf8().c_str()),Obj);
    EXPECT_EQ (DGNFILE_STATUS_Success, status) << status;
    ASSERT_TRUE( dgnProj != NULL);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, CreateWithInvalidName)
    {
    ScopedDgnHost           autoDgnHost;
    DgnProjectPtr      dgnProj;

    BeFileName dgndbFileName;
    BeTest::GetHost().GetOutputRoot (dgndbFileName);
    dgndbFileName.AppendToPath (L"MyTextDGNDBFILE.txt");
    if (BeFileName::DoesPathExist (dgndbFileName))
        BeFileName::BeDeleteFile (dgndbFileName);

    DgnFileStatus status;
    CreateProjectParams Obj;
    dgnProj = DgnProject::CreateProject(&status, BeFileName (dgndbFileName.GetNameUtf8().c_str()),Obj);
    EXPECT_EQ (DGNFILE_STATUS_Success, status) << status;
    ASSERT_TRUE( dgnProj != NULL);
    /////////It creates a DgnDbfile with .txt extension haveing success status needs to figure out is this right behave
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, FileNotFoundToOpen)
    {
    ScopedDgnHost autoDgnHost;
    DgnFileStatus status;
    CreateProjectParams Obj;
    DgnProjectPtr      dgnProj;

    BeFileName dgndbFileNotExist;
    BeTest::GetHost().GetOutputRoot (dgndbFileNotExist);
    dgndbFileNotExist.AppendToPath (L"MyFileNotExist.idgndb");

    dgnProj = DgnProject::OpenProject (&status, BeFileName (dgndbFileNotExist.GetNameUtf8().c_str()), DgnProject::OpenParams(Db::OPEN_Readonly));
    EXPECT_EQ (DGNOPEN_STATUS_FileNotFound, status) << status;
    ASSERT_TRUE( dgnProj == NULL);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adeel.Shoukat                      01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, OpenAlreadyOpen)
{
    ScopedDgnHost  autoDgnHost;

    WCharCP testFileName = L"ElementsSymbologyByLevel.idgndb";
    BeFileName sourceFile = DgnDbTestDgnManager::GetSeedFilePath(testFileName);

    BeFileName dgndbFileName;
    BeTest::GetHost().GetOutputRoot (dgndbFileName);
    dgndbFileName.AppendToPath (testFileName);

    BeFileName::BeCopyFile (sourceFile, dgndbFileName, false);

    DgnFileStatus status;
    DgnProjectPtr dgnProj = DgnProject::OpenProject (&status, dgndbFileName, DgnProject::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Exclusive));
    EXPECT_EQ (DGNFILE_STATUS_Success, status) << status;
    ASSERT_TRUE( dgnProj != NULL);

    // once a Db is opened for ReadWrite with exclusive access, it can't be opened, even for read.
    DgnProjectPtr dgnProj1 = DgnProject::OpenProject (&status, dgndbFileName, DgnProject::OpenParams(Db::OPEN_Readonly));
    EXPECT_EQ (DGNOPEN_STATUS_FileInUse, status) << status;
    ASSERT_TRUE( dgnProj1 == NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Julija.Suboc                08/13
* Covers GetAzimuth(), GetLatitude() and GetLongitude() in DgnProject
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, GetCoordinateSystemProperties)
    {
    ScopedDgnHost autoDgnHost;
    BeFileName fullFileName;
    TestDataManager::FindTestData (fullFileName, L"GeoCoordinateSystem.i.idgndb", DgnDbTestDgnManager::GetUtDatPath(__FILE__));
    //Open project
    DgnProjectPtr dgnProj;
    openProject (dgnProj, fullFileName, BeSQLite::Db::OPEN_Readonly);
    double azmth = dgnProj->Units().GetAzimuth();
    double azmthExpected = 178.2912;
    double eps = 0.0001;
    EXPECT_TRUE(fabs(azmthExpected - azmth) < eps )<<"Expected diffrent azimuth ";
    double latitude = dgnProj->Units().GetOriginLatitude();
    double latitudeExpected = 42.3413;
    EXPECT_TRUE(fabs(latitudeExpected - latitude) < eps)<<"Expected diffrent latitude ";
    double longitude = dgnProj->Units().GetOriginLongitude();
    double longitudeExpected = -71.0806;
    EXPECT_TRUE(fabs (longitudeExpected - longitude) < eps)<<"Expected diffrent longitude ";
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
        void getPropertiesInTable(DgnProjectPtr& project, PropertiesInTable& properties)
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
        void getPackageProperties(BeSQLite::Db& db, PropertiesInTable& properties, UInt64 embeddedFileId)
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
            UInt32 elmCount;
            UInt32 modelCount;
            size_t levelCount;
            size_t viewCount;
            size_t styleCount;  
            };
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                            Julija.Suboc                08/13
        +---------------+---------------+---------------+---------------+---------------+------*/
        void getProjectProperties(DgnProjectPtr& project, ProjectProperties& properties)
            {
            DgnModels& modelTable = project->Models();
            properties.elmCount = 0;
            properties.modelCount = 0;
            for (DgnModels::Iterator::Entry const& entry: modelTable.MakeIterator())
                {
                DgnModelP model = project->Models().GetModelById(entry.GetModelId());
                model->FillModel();
                properties.elmCount += model->GetElementCount();
                properties.modelCount++;
                }
            properties.viewCount = project->Views().MakeIterator().QueryCount();
            properties.levelCount = project->Levels().MakeIterator().QueryCount();
            DgnStyles& styleTable = project->Styles();
            properties.styleCount = styleTable.LineStyles().MakeIterator().QueryCount();
            }
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                            Julija.Suboc                08/13
        +---------------+---------------+---------------+---------------+---------------+------*/
        void compareProjectProperties(ProjectProperties& projProp, ProjectProperties& projPropV)
            {
            EXPECT_EQ(projProp.elmCount, projPropV.elmCount)<<"Element count does not match";
            EXPECT_EQ(projProp.modelCount, projPropV.modelCount)<<"Model count does not match";
            EXPECT_EQ(projProp.levelCount, projPropV.levelCount)<<"Level count does not match";
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
    DgnFileStatus status;
    DgnProjectPtr dgnProj;
    openProject (dgnProj, testFile, BeSQLite::Db::OPEN_ReadWrite);
    PropertiesInTable propertiesInTable;
    getPropertiesInTable(dgnProj, propertiesInTable);
    dgnProj->CloseDb();
    //Create package and open it for verification
    BeFileName packageFile = DgnDbTestDgnManager::GetOutputFilePath (L"package.db");
    CreatePackageParams createParams;
    createParams.SetOverwriteExisting(true);
    status =  DgnProjectPackage::CreatePackage(packageFile, testFile, createParams);
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
    DgnFileStatus status;
    DgnProjectPtr dgnProj;
    openProject (dgnProj, testFile, BeSQLite::Db::OPEN_ReadWrite);
    //Get properties for later verification
    ProjectProperties projectProp;
    getProjectProperties(dgnProj, projectProp);
    PropertiesInTable properties;
    getPropertiesInTable(dgnProj, properties);
    dgnProj->CloseDb();
    //Create package
    BeFileName packageFile = DgnDbTestDgnManager::GetOutputFilePath (L"package.db");
    CreatePackageParams createParams;
    createParams.SetOverwriteExisting(true);
    status =  DgnProjectPackage::CreatePackage(packageFile, testFile, createParams);
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
    status = DgnProjectPackage::Extract(dbResult, extractedFileDir.GetNameUtf8().c_str(), fileName.c_str(), packageFile, true);
    EXPECT_EQ(DGNFILE_STATUS_Success, status);
    EXPECT_TRUE(BE_SQLITE_OK == dbResult);
    //Open project that was extracted from package  
    DgnProjectPtr dgnProjV;
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
    DgnFileStatus status;
    DgnProjectPtr dgnProj;
    openProject (dgnProj, testFile, BeSQLite::Db::OPEN_ReadWrite);
    //Get properties for later verification
    ProjectProperties projectProp;
    getProjectProperties(dgnProj, projectProp);
    PropertiesInTable properties;
    getPropertiesInTable(dgnProj, properties);
    dgnProj->CloseDb();
    
    //Create package
    BeFileName packageFile = DgnDbTestDgnManager::GetOutputFilePath (L"package.db");
    CreatePackageParams createParams;
    createParams.SetOverwriteExisting(true);
    status =  DgnProjectPackage::CreatePackage(packageFile, testFile, createParams);
    EXPECT_EQ (DGNFILE_STATUS_Success, status);
    //Extract file from package
    DbResult dbResult;
    BeFileName extractedFile = DgnDbTestDgnManager::GetOutputFilePath (L"extractedUsingDefaults.idgndb");
    status = DgnProjectPackage::ExtractUsingDefaults(dbResult, extractedFile, packageFile, true);
    EXPECT_EQ(DGNFILE_STATUS_Success, status);
    EXPECT_TRUE(BE_SQLITE_OK == dbResult);
   
    //Open file for verification
    DgnProjectPtr dgnProjV;
    openProject (dgnProjV, extractedFile, BeSQLite::Db::OPEN_ReadWrite);
   //Verify that properties did not change
    ProjectProperties projectPropV;
    getProjectProperties(dgnProjV, projectPropV);
    compareProjectProperties(projectProp, projectPropV);
    PropertiesInTable propertiesV;
    getPropertiesInTable(dgnProjV, propertiesV);
    compareProperties(properties, propertiesV);
    }
