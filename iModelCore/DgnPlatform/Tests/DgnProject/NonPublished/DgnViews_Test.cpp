/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnViews_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>
#include <Bentley/bset.h>

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// @bsiclass                                                    Majd.Uddin   04/12
//=======================================================================================
struct TestViewProperties
    {
    public:
        DgnViewId       tvId;
        WString         tvName;
        DgnViewType     tvViewType;

        void SetTestViewProperties (WString Name, DgnViewType viewType)
            {
            tvName = Name;
            tvViewType = viewType;
            };
        void IsEqual (TestViewProperties testView)
            {
            EXPECT_STREQ (tvName.c_str(), testView.tvName.c_str()) << "Names don't match";
            EXPECT_EQ (tvViewType, testView.tvViewType) << "View Types don't match";
            };
    };

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnViews
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnViewsTest : public ::testing::Test
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
void DgnViewsTest::SetupProject (WCharCP projFile, FileOpenMode mode)
    {
    DgnDbTestDgnManager tdm (projFile, __FILE__, mode);
    project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Work with Views
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, WorkWithViewTable)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    //Get views
    DgnViews viewTable = project->Views();
    DgnViews::Iterator iter = viewTable.MakeIterator();
    ASSERT_EQ (6, iter.QueryCount()) <<"The expected view count is ... where as it is: " << iter.QueryCount();

    //Iterate through each view and make sure they have correct information
    TestViewProperties fileViews[6], testView;
    fileViews[0].SetTestViewProperties (L"View 1, Default", DGNVIEW_TYPE_Drawing);
    fileViews[1].SetTestViewProperties (L"View 2, Default", DGNVIEW_TYPE_Drawing);
    fileViews[2].SetTestViewProperties (L"View 1, Model2d", DGNVIEW_TYPE_Drawing);
    fileViews[3].SetTestViewProperties (L"View 2, Model2d", DGNVIEW_TYPE_Drawing);
    fileViews[4].SetTestViewProperties (L"View 1, Default [master.i.dgn]", DGNVIEW_TYPE_Drawing);
    fileViews[5].SetTestViewProperties (L"View 2, Default [master.i.dgn]", DGNVIEW_TYPE_Drawing);

    int i = 0;
    for (auto const& entry : iter)
        {
        WString entryNameW (entry.GetName(), true);
        testView.SetTestViewProperties (entryNameW.c_str(), entry.GetDgnViewType());
        testView.IsEqual (fileViews[i]);
        i++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Delete view test
* @bsimethod                               Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, DeleteView)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    //Get views
    DgnViews viewTable = project->Views();

    DgnViewId viewId(1);
    DgnViews::View view = viewTable.QueryViewById(viewId);
    EXPECT_TRUE(view.IsValid());

    EXPECT_EQ(BE_SQLITE_DONE, viewTable.DeleteView(viewId));
    
    view = viewTable.QueryViewById(viewId);
    EXPECT_FALSE(view.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* Set name test view
* @bsimethod                               Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, SetViewName)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    //Get views
    DgnViews viewTable = project->Views();

    DgnViewId viewId(1);
    DgnViews::View view = viewTable.QueryViewById(viewId);
    EXPECT_TRUE(view.IsValid());
    
    EXPECT_STRNE("TestView", view.GetName());
    view.SetName("TestView");
    EXPECT_STREQ("TestView", view.GetName());
    
    //Set selector id, because selectorId from database is not valid
    DgnModelSelectorId selectorId(1);
    view.SetSelectorId(selectorId);

    EXPECT_EQ(BE_SQLITE_DONE, viewTable.UpdateView(view));
    
    view = viewTable.QueryViewById(viewId);
    EXPECT_TRUE(view.IsValid());
    EXPECT_STREQ("TestView", view.GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, ViewSaveAs)
    {
    SetupProject(L"3dMetricGeneral.idgndb", OPENMODE_READWRITE);

    ViewControllerPtr controller = project->Views().LoadViewController(DgnViewId(1));
    EXPECT_EQ(BE_SQLITE_OK, controller->SaveAs("NewView2"));
    EXPECT_TRUE(DgnViewId(2) == controller->GetViewId());

    DgnViewId view3Id;
    EXPECT_EQ(BE_SQLITE_OK, controller->SaveTo("NewView3", view3Id));
    EXPECT_TRUE(DgnViewId(2) == controller->GetViewId());

    project->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* Iterator entry properties test
* @bsimethod                               Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, IteratorEntryProperties)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    //Get views
    DgnViews viewTable = project->Views();

    DgnViewId viewId(1);
    DgnViews::View view = viewTable.QueryViewById(viewId);
    EXPECT_TRUE(view.IsValid());
    
    DgnModelSelectorId selectorId(1);
    DgnModelId modelId(0);

    view.SetSelectorId(selectorId);
    view.SetBaseModelId(modelId);

    EXPECT_EQ(BE_SQLITE_DONE, viewTable.UpdateView(view));

    DgnViews::Iterator iter = viewTable.MakeIterator(DGNVIEW_TYPE_All);
    iter.Params().SetWhere(" AND Id = 1");
    DgnViews::Iterator::Entry entry = iter.begin();

    EXPECT_EQ(1, entry.GetSelectorId().GetValue());
    EXPECT_EQ(0, entry.GetBaseModelId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* Insert a View 
* @bsimethod                               Ahmed.Rizwan                    10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, InsertView)
    {
    SetupProject (L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnModelSelectorId selector = DgnModelSelectorId (2);
    // Get views
    DgnViews viewTable = project->Views ();
    DgnViewId viewId (4);
    // Create a new view
    DgnViews::View tempView = DgnViews::View (DGNVIEW_TYPE_Physical,
                                             "",
                                             DgnModelId (2),
                                             "TestView",
                                             NULL,
                                             DgnModelSelectorId (2),
                                             DGNVIEW_SOURCE_User,
                                             DgnViewId (5)
                                            );

    // Insert 
    ASSERT_EQ (BE_SQLITE_OK, viewTable.InsertView (tempView)) << "Unable to insert View";
    tempView = viewTable.QueryViewById (DgnViewId (5));
    EXPECT_TRUE (tempView.IsValid ()) << "View not found";
    // Verify Properties
    TestViewProperties originalView, testView;
    originalView.SetTestViewProperties (L"TestView", DGNVIEW_TYPE_Physical);
    WString entryNameW (tempView.GetName (), true);
    testView.SetTestViewProperties (entryNameW.c_str (), tempView.GetDgnViewType ());
    testView.IsEqual (originalView);

    }

/*---------------------------------------------------------------------------------**//**
* Insert a View after it is deleted
* @bsimethod                               Ahmed.Rizwan                    10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, InsertDeletedView)
    {
    SetupProject (L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnModelSelectorId selector = DgnModelSelectorId (2);
    // Get views
    DgnViews viewTable = project->Views ();
    DgnViewId viewId (4);
    // Get an existing view
    DgnViews::View exisitingView = viewTable.QueryViewById (viewId);
    ASSERT_TRUE (exisitingView.IsValid ()) << "Unable to find the view";
    // Delete it
    ASSERT_EQ (BE_SQLITE_DONE, viewTable.DeleteView (viewId)) << "Unable to delete the View";

    // Verify that it is deleted
    EXPECT_TRUE (!viewTable.QueryViewById (viewId).IsValid ()) << "View not deleted";
    // Insert it back
    ASSERT_EQ (BE_SQLITE_OK, viewTable.InsertView (exisitingView)) << "Unable to insert View";
    // Verify that it is inserted
    exisitingView = viewTable.QueryViewById (viewId);
    EXPECT_TRUE (exisitingView.IsValid ()) << "View not found";
    // Verify properties
    TestViewProperties originalView, testView;
    originalView.SetTestViewProperties (L"View 2, Model2d", DGNVIEW_TYPE_Drawing);
    WString entryNameW (exisitingView.GetName (), true);
    testView.SetTestViewProperties (entryNameW.c_str (), exisitingView.GetDgnViewType ());
    testView.IsEqual (originalView);

    }

/*---------------------------------------------------------------------------------**//**
* Insert an existing View
* @bsimethod                               Ahmed.Rizwan                    10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, InsertExistingView)
    {
    SetupProject (L"ElementsSymbologyByLevel.idgndb", OPENMODE_READWRITE);

    DgnModelSelectorId selector = DgnModelSelectorId (2);
    // Get views
    DgnViews viewTable = project->Views ();
    DgnViewId viewId (4);
    // Create a new view
    DgnViews::View tempView = DgnViews::View (DGNVIEW_TYPE_Physical,
                                             "",
                                             DgnModelId (2),
                                             "TestView",
                                             0,
                                             DgnModelSelectorId (2),
                                             DGNVIEW_SOURCE_User,
                                             DgnViewId (5)
                                            );

    // Insert 
    ASSERT_EQ (BE_SQLITE_OK, viewTable.InsertView (tempView)) << "Unable to insert View";
    tempView = viewTable.QueryViewById (DgnViewId (5));
    EXPECT_TRUE (tempView.IsValid ()) << "View not found";
    EXPECT_STREQ ("TestView", tempView.GetName ()) << "View name does not match";
    // Verify second insertion fails
    ASSERT_EQ (BE_SQLITE_CONSTRAINT_UNIQUE, viewTable.InsertView (tempView)) << "Second view should not be inserted";
    tempView = viewTable.QueryViewById (DgnViewId (6));
    EXPECT_FALSE (tempView.IsValid ()) << "View should not be found";

    }

/*---------------------------------------------------------------------------------**//**
* Iterator entry properties test
* @bsimethod                                                    Sam.Wilson      06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, GetRelatedDrawings)
    {
    DgnDbTestDgnManager tdm (L"dv.idgndb", __FILE__, OPENMODE_READONLY);
    auto project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    bset<DgnModelId> pmodels;
    pmodels.insert (DgnModelId(0));

    BeSQLite::Statement selectDrawingsForModels;
    ASSERT_TRUE( project->ViewGeneratedDrawings().QueryViewsOfGeneratedModels (selectDrawingsForModels, pmodels, "Id,Name,Descr") == BeSQLite::BE_SQLITE_OK );

    size_t  drawingCount = 0;
    while (selectDrawingsForModels.Step() == BeSQLite::DbResult::BE_SQLITE_ROW)
        {
        DgnViewId viewId = selectDrawingsForModels.GetValueId<DgnViewId> (0);
        printf ("%lld %s %s\n", viewId.GetValue(), selectDrawingsForModels.GetValueText(1), selectDrawingsForModels.GetValueText(2));
        ++drawingCount;
        }

    ASSERT_EQ(drawingCount,1);
    }