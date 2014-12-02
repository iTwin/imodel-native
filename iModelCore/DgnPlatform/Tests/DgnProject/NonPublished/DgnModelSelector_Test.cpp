/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnModelSelector_Test.cpp $
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
struct TestModelSelectorProperties
    {
    public:
        DgnModelSelectorId      tmsId;
        WString                 tmsName;

        void SetTestModelSelectorProperties (WString Name)
            {
            tmsName = Name;
            };
        void IsEqual (TestModelSelectorProperties Model)
            {
            EXPECT_STREQ (tmsName.c_str(), Model.tmsName.c_str()) << "Names don't match";
            };
    };

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnColors
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnModelSelector : public ::testing::Test
    {
    public:
        ScopedDgnHost           m_host;
        DgnProjectPtr      project;

        void SetupProject (WCharCP projFile, FileOpenMode mode);
    };

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .dgndb project file
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModelSelector::SetupProject (WCharCP projFile, FileOpenMode mode)
    {
    
    DgnDbTestDgnManager tdm (projFile, __FILE__, mode);
    project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Work with ModelSelector Table
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelSelector, WorkWithDgnModelSelector)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnModelSelectors modTable = project->ModelSelectors();
    DgnModelSelectors::Iterator modIter = modTable.MakeIterator();
    EXPECT_EQ (2, modIter.QueryCount());

    TestModelSelectorProperties models[2], testModel;
    models[0].SetTestModelSelectorProperties (L"Default");
    models[1].SetTestModelSelectorProperties (L"Attachments from Default");

    //Checking Properties
    int i = 0;
    FOR_EACH (DgnModelSelectors::Iterator::Entry const& entry, modIter)
        {
        WString entryNameW (entry.GetName(), true); // string conversion
        testModel.SetTestModelSelectorProperties (entryNameW.c_str());
        testModel.IsEqual (models[i]);
        i++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Delete model selector
* @bsimethod                                    Algirdas.Mikoliunas               02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelSelector, DeleteModelSelector)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnModelSelectorId modelSelectorId(1);

    DgnModelSelectors modTable = project->ModelSelectors();
    DgnModelSelectors::Selector querySelector = modTable.QuerySelectorById(modelSelectorId);
    EXPECT_TRUE(querySelector.IsValid());

    DgnModelSelectors::Iterator iter = modTable.MakeIterator();
    EXPECT_EQ(2, iter.QueryCount());
    
    EXPECT_EQ(BE_SQLITE_DONE, modTable.DeleteSelector(modelSelectorId));

    querySelector = modTable.QuerySelectorById(modelSelectorId);
    EXPECT_FALSE(querySelector.IsValid());

    DgnModelSelectors::Iterator iterDeleted = modTable.MakeIterator();
    EXPECT_EQ(1, iterDeleted.QueryCount());
    }

/*---------------------------------------------------------------------------------**//**
* Delete non existing model selector
* @bsimethod                                    Algirdas.Mikoliunas               02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelSelector, DeleteNonExistingModelSelector)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);
    
    DgnModelSelectorId modelSelectorId(5);
    DgnModelSelectors modTable = project->ModelSelectors();

    DgnModelSelectors::Iterator iter = modTable.MakeIterator();
    EXPECT_EQ(2, iter.QueryCount());

    EXPECT_EQ(BE_SQLITE_DONE, modTable.DeleteSelector(modelSelectorId));

    DgnModelSelectors::Iterator iterDeleted = modTable.MakeIterator();
    EXPECT_EQ(2, iterDeleted.QueryCount());
    }

/*---------------------------------------------------------------------------------**//**
* Query model selector id
* @bsimethod                                    Algirdas.Mikoliunas               02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelSelector, QuerySelectorId)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnModelSelectors modTable = project->ModelSelectors();
    DgnModelSelectorId modelSelectorId = modTable.QuerySelectorId("Default");

    EXPECT_EQ(1, modelSelectorId.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* Get iterator properties
* @bsimethod                                    Algirdas.Mikoliunas               02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelSelector, GetIteratorProperties)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnModelSelectors modTable = project->ModelSelectors();
    DgnModelSelectors::Iterator iter = modTable.MakeIterator();
    DgnModelSelectors::Iterator::Entry entry = iter.begin();
    
    EXPECT_STREQ("default model selector", entry.GetDescription());
    EXPECT_EQ(1, entry.GetId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* Add/remove model from selector
* @bsimethod                                    Algirdas.Mikoliunas               02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelSelector, AddRemoveModelSelector)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);
    
    DgnModelSelectors modTable = project->ModelSelectors();
    DgnModelSelectorId modelSelectorId(1);
    DgnModelId modelId(0);

    EXPECT_EQ(BE_SQLITE_OK, modTable.AddModelToSelector(modelSelectorId, modelId));

    DgnModelSelection modelSelection(*project, modelSelectorId);
    EXPECT_EQ(SUCCESS, modelSelection.Load());
    EXPECT_EQ(1, modelSelection.GetExplictModelCount());

    EXPECT_EQ(BE_SQLITE_DONE, modTable.RemoveModelFromSelector(modelSelectorId, modelId));

    DgnModelSelection modelSelectionAfterRemove(*project, modelSelectorId);

    // This would normally trigger an assertion failure.
    BeTest::SetFailOnAssert (false);
    modelSelectionAfterRemove.Load();
    BeTest::SetFailOnAssert (true);

    EXPECT_EQ(0, modelSelectionAfterRemove.GetExplictModelCount());
    EXPECT_FALSE(modelSelectionAfterRemove.HasExplicitModel(modelId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Julija.Suboc                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnModelSelector, DropExplicitModel)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);
    
    DgnModelSelectors modTable = project->ModelSelectors();
    DgnModelSelectorId modelSelectorId(1);
    DgnModelId modelId(0);

    EXPECT_EQ(BE_SQLITE_OK, modTable.AddModelToSelector(modelSelectorId, modelId));

    DgnModelSelection modelSelection(*project, modelSelectorId);
    ASSERT_EQ(SUCCESS, modelSelection.Load())<<"Failed to load selection";
    ASSERT_EQ(1, modelSelection.GetExplictModelCount())<<"There are more models in selection than expected";

    ASSERT_EQ(SUCCESS, modelSelection.DropExplicitModel(modelId));
    ASSERT_EQ(0, modelSelection.GetExplictModelCount())<<"Model was not deleted from selection";

    DgnModelSelection modelSelectionAfterRemove(*project, modelSelectorId);
    // This would normally trigger an assertion failure.
    BeTest::SetFailOnAssert (false);
    modelSelectionAfterRemove.Load();
    BeTest::SetFailOnAssert (true);

    EXPECT_EQ(0, modelSelectionAfterRemove.GetExplictModelCount());
    EXPECT_FALSE(modelSelectionAfterRemove.HasExplicitModel(modelId));
    }
