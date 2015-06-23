/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnViews_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
        DgnDbPtr      project;

        void SetupProject (WCharCP projFile, Db::OpenMode mode);
    };

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .dgndb project file
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewsTest::SetupProject (WCharCP projFile, Db::OpenMode mode)
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
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OPEN_ReadWrite);

    //Get views
    DgnViews& viewTable = project->Views ();
    DgnViews::Iterator iter = viewTable.MakeIterator();
    ASSERT_EQ (4, iter.QueryCount()) <<"The expected view count is 6 where as it is: " << iter.QueryCount();

    //Iterate through each view and make sure they have correct information
    TestViewProperties fileViews[4], testView;
    fileViews[0].SetTestViewProperties (L"Default - View 1", DgnViewType::Drawing);
    fileViews[1].SetTestViewProperties (L"Default - View 2", DgnViewType::Drawing);
    fileViews[2].SetTestViewProperties (L"Model2d Views - View 1", DgnViewType::Drawing);
    fileViews[3].SetTestViewProperties (L"Model2d Views - View 2", DgnViewType::Drawing);

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
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OPEN_ReadWrite);

    //Get views
    DgnViews& viewTable = project->Views ();

    DgnViewId viewId((int64_t)1);
    DgnViews::View view = viewTable.QueryView(viewId);
    EXPECT_TRUE(view.IsValid());

    EXPECT_EQ(BE_SQLITE_DONE, viewTable.Delete(viewId));
    
    view = viewTable.QueryView(viewId);
    EXPECT_FALSE(view.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* Set name test view
* @bsimethod                               Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, SetViewName)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OPEN_ReadWrite);

    //Get views
    DgnViews& viewTable = project->Views ();

    DgnViewId viewId((int64_t)1);
    DgnViews::View view = viewTable.QueryView(viewId);
    EXPECT_TRUE(view.IsValid());
    
    EXPECT_STRNE("TestView", view.GetName());
    view.SetName("TestView");
    EXPECT_STREQ("TestView", view.GetName());
    
    EXPECT_EQ(BE_SQLITE_DONE, viewTable.Update(view));
    
    view = viewTable.QueryView(viewId);
    EXPECT_TRUE(view.IsValid());
    EXPECT_STREQ("TestView", view.GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, ViewSaveAs)
    {
    SetupProject(L"3dMetricGeneral.idgndb", Db::OPEN_ReadWrite);

    ViewControllerPtr controller = project->Views().LoadViewController(DgnViewId((int64_t)1));
    EXPECT_EQ(BE_SQLITE_OK, controller->SaveAs("NewView2"));
    EXPECT_TRUE(DgnViewId((int64_t)2) == controller->GetViewId());

    DgnViewId view3Id;
    EXPECT_EQ(BE_SQLITE_OK, controller->SaveTo("NewView3", view3Id));
    EXPECT_TRUE(DgnViewId((int64_t)2) == controller->GetViewId());

    project->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* Iterator entry properties test
* @bsimethod                               Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, IteratorEntryProperties)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OPEN_ReadWrite);

    //Get views
    DgnViews& viewTable = project->Views();

    DgnViewId viewId((int64_t)1);
    DgnViews::View view = viewTable.QueryView(viewId);
    EXPECT_TRUE(view.IsValid());
    
    DgnModelId modelId((int64_t)1);
    view.SetBaseModelId(modelId);

    EXPECT_EQ(BE_SQLITE_DONE, viewTable.Update(view));

    DgnViews::Iterator iter = viewTable.MakeIterator((int) DgnViewType::All);
    iter.Params().SetWhere(" AND Id = 1");
    DgnViews::Iterator::Entry entry = iter.begin();

    EXPECT_EQ(1, entry.GetBaseModelId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* Insert a View 
* @bsimethod                               Ahmed.Rizwan                    10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, InsertView)
    {
    SetupProject (L"ElementsSymbologyByLevel.idgndb", Db::OPEN_ReadWrite);

    // Get views
    DgnViews& viewTable = project->Views ();
    DgnViewId viewId ((int64_t)4);
    // Create a new view
    DgnViews::View tempView = DgnViews::View (DgnViewType::Physical,
                                             DgnClassId(project->Schemas().GetECClassId("dgn","PhysicalView")),
                                             DgnModelId ((int64_t)2),
                                             "TestView",
                                             NULL,
                                             DgnViewSource::User,
                                             DgnViewId ((int64_t)5)
                                            );

    // Insert 
    ASSERT_EQ (BE_SQLITE_OK, viewTable.Insert(tempView)) << "Unable to insert View";
    tempView = viewTable.QueryView (DgnViewId ((int64_t)5));
    EXPECT_TRUE (tempView.IsValid ()) << "View not found";
    // Verify Properties
    TestViewProperties originalView, testView;
    originalView.SetTestViewProperties (L"TestView", DgnViewType::Physical);
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
    SetupProject (L"ElementsSymbologyByLevel.idgndb", Db::OPEN_ReadWrite);

    // Get views
    DgnViews& viewTable = project->Views ();
    DgnViewId viewId ((int64_t)4);
    // Get an existing view
    DgnViews::View exisitingView = viewTable.QueryView(viewId);
    ASSERT_TRUE (exisitingView.IsValid ()) << "Unable to find the view";
    // Delete it
    ASSERT_EQ (BE_SQLITE_DONE, viewTable.Delete(viewId)) << "Unable to delete the View";

    // Verify that it is deleted
    EXPECT_TRUE (!viewTable.QueryView (viewId).IsValid ()) << "View not deleted";
    // Insert it back
    ASSERT_EQ (BE_SQLITE_OK, viewTable.Insert(exisitingView)) << "Unable to insert View";
    // Verify that it is inserted
    exisitingView = viewTable.QueryView (viewId);
    EXPECT_TRUE (exisitingView.IsValid ()) << "View not found";
    // Verify properties
    TestViewProperties originalView, testView;
    originalView.SetTestViewProperties (L"Model2d Views - View 2", DgnViewType::Drawing);
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
    SetupProject (L"ElementsSymbologyByLevel.idgndb", Db::OPEN_ReadWrite);

    // Get views
    DgnViews& viewTable = project->Views ();
    DgnViewId viewId ((int64_t)4);
    // Create a new view
    DgnViews::View tempView = DgnViews::View (DgnViewType::Physical,
                                             DgnClassId(project->Schemas().GetECClassId("dgn","PhysicalView")),
                                             DgnModelId ((int64_t)2),
                                             "TestView",
                                             0,
                                             DgnViewSource::User,
                                             DgnViewId ((int64_t)5)
                                            );

    // Insert 
    ASSERT_EQ (BE_SQLITE_OK, viewTable.Insert(tempView)) << "Unable to insert View";
    tempView = viewTable.QueryView (DgnViewId ((int64_t)5));
    EXPECT_TRUE (tempView.IsValid ()) << "View not found";
    EXPECT_STREQ ("TestView", tempView.GetName ()) << "View name does not match";
    // Verify second insertion fails
    ASSERT_EQ (BE_SQLITE_CONSTRAINT_UNIQUE, viewTable.Insert(tempView)) << "Second view should not be inserted";
    tempView = viewTable.QueryView (DgnViewId ((int64_t)6));
    EXPECT_FALSE (tempView.IsValid ()) << "View should not be found";

    }
