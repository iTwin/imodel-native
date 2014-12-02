/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ForeignFormat/NonPublished/DgnModelTable_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ForeignFormatTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>
#include <DgnPlatform/ForeignFormat/DgnV8ProjectImporter.h>

USING_NAMESPACE_BENTLEY_SQLITE 
USING_NAMESPACE_FOREIGNFORMAT
USING_NAMESPACE_DGNV8

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnModels
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnModelTableTests : public ::testing::Test
    {
    public:
        ScopedDgnHost m_host;
        DgnProjectPtr m_proj;

        void SetupProject (WCharCP projFile, WCharCP seedFile);
        void SetupProject (WCharCP projFile, WCharCP testFile, BeSQLite::Db::OpenMode mode);
        void VerifyModelCount (int count);

    };

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .dgndb project file after copying it to out
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModelTableTests::SetupProject (WCharCP projFile, WCharCP testFile, BeSQLite::Db::OpenMode mode)
    {
    BeFileName outFileName;
    ASSERT_EQ (SUCCESS, DgnDbTestDgnManager::GetTestDataOut (outFileName, projFile, testFile, __FILE__));
    DgnFileStatus result;
    m_proj = DgnProject::OpenProject (&result, outFileName, DgnProject::OpenParams(mode));
    ASSERT_TRUE (m_proj.IsValid());
    ASSERT_TRUE( result == DGNFILE_STATUS_Success);
    }
/*---------------------------------------------------------------------------------**//**
* set up method that creates a new DgnProject and imports the dgn file
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModelTableTests::SetupProject (WCharCP projFile, WCharCP seedFile)
    {
    BeFileName projectFileName;
    BeTest::GetHost().GetOutputRoot (projectFileName);
    projectFileName.AppendToPath (projFile);
    
    //Delete if it exists
    BeFileName::BeDeleteFile (projectFileName);
    //Create new project
    CreateProjectParams params;
    DgnFileStatus result;
    m_proj = DgnProject::CreateProject (&result, projectFileName, params);
    //Verify new project
    ASSERT_TRUE(m_proj.IsValid());
    ASSERT_TRUE( result == DGNFILE_STATUS_Success);
    Utf8String projectFileNameUtf8 (projectFileName.GetName());
    ASSERT_TRUE( projectFileNameUtf8 == Utf8String(m_proj->GetDbFileName()));

    //Import the Dgn file
    BeFileName fullFileName;
    DgnDbTestDgnManager::FindTestData (fullFileName, seedFile, __FILE__);
    DgnV8ProjectImporter publisher (*m_proj, DgnImportParams());
    publisher.SetSeedFile(fullFileName);
    publisher.PerformImport();
    }

/*---------------------------------------------------------------------------------**//**
* Verify the count of models in the table
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModelTableTests::VerifyModelCount (int count)
    {
    int i = 0;
    FOR_EACH (DgnModels::Iterator::Entry const& entry, m_proj->Models().MakeIterator())
        {
        EXPECT_TRUE (entry.GetModelId().IsValid()) << "Model Id is not Valid";
        i++;
        }
    EXPECT_EQ (count, i) << "The actual model count is: " << i << "where as expected was: " << count;

    //A second check
    EXPECT_EQ (count, m_proj->Models().MakeIterator().QueryCount());
    }

/*---------------------------------------------------------------------------------**//**
* Add new models to the table
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTableTests, AddModel)
    {
    SetupProject (L"ElementsSymbologyByLevel.idgndb", L"AddModel.idgndb", BeSQLite::Db::OPEN_ReadWrite);
    VerifyModelCount (3);

    //Now add a new model
    DgnModels::Model row;
    row.SetName ("TestModel");
    row.SetModelType (DgnModelType::Sheet);
    EXPECT_EQ (BE_SQLITE_OK, m_proj->Models().InsertModel(row));

    //Verify that this model can be found
    VerifyModelCount (4);
    //Search this model
    DgnModelId modelId = m_proj->Models().QueryModelId("TestModel");
    EXPECT_TRUE (modelId.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* Remove a model from the table
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTableTests, DeleteModel)
    {
    SetupProject (L"ElementsSymbologyByLevel.idgndb", L"DeleteModel.idgndb", BeSQLite::Db::OPEN_ReadWrite);
    VerifyModelCount (3);

    //Now delete a model
    DgnModelId modelId = m_proj->Models().QueryModelId("Model2d");
    EXPECT_TRUE (modelId.IsValid());

    EXPECT_EQ (SUCCESS, m_proj->Models().DeleteModel(modelId));

    //Verify that this model can be found
    VerifyModelCount (2);
    //Search this model
    modelId = m_proj->Models().QueryModelId("Model2d");
    EXPECT_TRUE (!modelId.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* Get model properties from the table
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTableTests, GetModelDetails)
    {
    SetupProject (L"ElementsSymbologyByLevel.idgndb", L"GetModelDetails.idgndb", BeSQLite::Db::OPEN_Readonly);
    VerifyModelCount (3);

    DgnModelId modelId = m_proj->Models().QueryModelId("Model2d");
    EXPECT_TRUE (modelId.IsValid());

    //Get details from this modelId
    Utf8String modelName;
    EXPECT_EQ (SUCCESS, m_proj->Models().GetModelName(modelName, modelId));
    EXPECT_TRUE (0 == strcmp ("Model2d", modelName.c_str()));
    DgnModels::Model row;
    EXPECT_EQ (SUCCESS, m_proj->Models().QueryModelById (&row, modelId));

    EXPECT_TRUE (row.GetId().IsValid());
    EXPECT_TRUE (0 == strcmp ("Model2d", row.GetNameCP()));
    EXPECT_TRUE (0 == strcmp ("", row.GetDescription()));
    EXPECT_TRUE (DgnModelType::Drawing == row.GetModelType());
    EXPECT_EQ (255, row.GetVisibility());
    EXPECT_TRUE (row.IsIterable());
    EXPECT_TRUE (row.InModelGui());
    EXPECT_FALSE (row.Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* Change model properties from the table
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTableTests, ChangeModelDetails)
    {
    SetupProject (L"ElementsSymbologyByLevel.idgndb", L"ChangeModelDetails.idgndb", BeSQLite::Db::OPEN_ReadWrite);
    VerifyModelCount (3);

    DgnModelId modelId = m_proj->Models().QueryModelId("Model2d");
    EXPECT_TRUE (modelId.IsValid());

    //Change some of the values
    DgnModels::Model row;
    EXPECT_EQ (SUCCESS, m_proj->Models().QueryModelById (&row, modelId));
    
    row.SetName ("Model3d");
    row.SetModelType (DgnModelType::Physical);
    row.SetDescription ("This is a 3d Model now");

    //Verify values are updated
    EXPECT_TRUE (0 == strcmp ("Model3d", row.GetNameCP()));
    EXPECT_TRUE (DgnModelType::Physical == row.GetModelType());
    EXPECT_TRUE (row.Is3d());
    EXPECT_TRUE (0 == strcmp (row.GetDescription(), "This is a 3d Model now"));
    }

/*---------------------------------------------------------------------------------**//**
* Load a model from ModelTable
* @bsimethod                                    Majd.Uddin                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelTableTests, LoadModel)
    {
    SetupProject (L"ElementsSymbologyByLevel.idgndb", L"LoadModel.idgndb", BeSQLite::Db::OPEN_ReadWrite);

    DgnModelId modelId = m_proj->Models().QueryModelId("Model2d");
    EXPECT_TRUE (modelId.IsValid());

    //Now load this model and do some thing
    StatusInt error;
    DgnModelP model1 = m_proj->Models().GetAndFillModelById (&error, modelId);
    EXPECT_TRUE (model1 != NULL);
    model1->FillSections (DgnModelSections::Model);
    }

