/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnViews_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/ColorUtil.h>
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
    DgnDbTestDgnManager tdm (projFile, __FILE__, mode, false);
    project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Work with Views
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, WorkWithViewTable)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);

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
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);

    //Get views
    DgnViews& viewTable = project->Views ();

    DgnViewId viewId((uint64_t)1);
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
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);

    //Get views
    DgnViews& viewTable = project->Views ();

    DgnViewId viewId((uint64_t)1);
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
    SetupProject(L"3dMetricGeneral.idgndb", Db::OpenMode::ReadWrite);

    ViewControllerPtr controller = project->Views().LoadViewController(DgnViewId((uint64_t)1));
    EXPECT_EQ(BE_SQLITE_OK, controller->SaveAs("NewView2"));
    EXPECT_TRUE(DgnViewId((uint64_t)2) == controller->GetViewId());

    DgnViewId view3Id;
    EXPECT_EQ(BE_SQLITE_OK, controller->SaveTo("NewView3", view3Id));
    EXPECT_TRUE(DgnViewId((uint64_t)2) == controller->GetViewId());

    project->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* Iterator entry properties test
* @bsimethod                               Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, IteratorEntryProperties)
    {
    SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);

    //Get views
    DgnViews& viewTable = project->Views();

    DgnViewId viewId((uint64_t)1);
    DgnViews::View view = viewTable.QueryView(viewId);
    EXPECT_TRUE(view.IsValid());
    
    DgnModelId modelId((uint64_t)1);
    view.SetBaseModelId(modelId);

    EXPECT_EQ(BE_SQLITE_DONE, viewTable.Update(view));

    DgnViews::Iterator iter = viewTable.MakeIterator((int) DgnViewType::All);
    iter.Params().SetWhere(" AND Id = 1");
    DgnViews::Iterator::Entry entry = iter.begin();

    EXPECT_EQ(1, entry.GetBaseModelId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* CRUD
* @bsimethod                               Umar Hayagt                    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, CRUD)
    {
    SetupProject (L"3dMetricGeneral.idgndb", Db::OpenMode::ReadWrite);

    // Get views
    DgnViews& viewTable = project->Views ();
    // Create a new view
    DgnViews::View tempView = DgnViews::View (DgnViewType::Physical,
                                             DgnClassId(project->Schemas().GetECClassId("dgn","PhysicalView")),
                                             DgnModelId ((uint64_t)2),
                                             "TestView",
                                             "Test Description",
                                             DgnViewSource::User
                                            );
    
    DgnViews::View tempView2 = DgnViews::View (DgnViewType::Drawing,
                                             DgnClassId(project->Schemas().GetECClassId("dgn","PhysicalView")),
                                             DgnModelId ((uint64_t)1),
                                             "TestDrawingView",
                                             "TestDrawingView Description",
                                             DgnViewSource::Private
                                            );

    // Insert 
    ASSERT_EQ (BE_SQLITE_OK, viewTable.Insert(tempView)) << "Unable to insert View";
    DgnViewId viewId = tempView.GetId();
    ASSERT_TRUE(viewId.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, viewTable.Insert(tempView2)) << "Unable to insert View";
    DgnViewId viewId2 = tempView2.GetId();
    ASSERT_TRUE(viewId2.IsValid());

    //  Query
    //
    ASSERT_TRUE( viewId == viewTable.QueryViewId("TestView"));
    

    //  Iterate
    //
    for (DgnViews::Iterator::Entry entry : viewTable.MakeIterator())
        {
        if (entry.GetDgnViewId() == viewId)
            {
            DgnViews::View toFind = viewTable.QueryView(viewId);
            EXPECT_TRUE(tempView.GetId() == toFind.GetId());
            EXPECT_TRUE(tempView.GetBaseModelId() == toFind.GetBaseModelId());
            EXPECT_TRUE(tempView.GetClassId() == toFind.GetClassId());
            EXPECT_TRUE(tempView.GetDgnViewSource() == toFind.GetDgnViewSource());
            EXPECT_STREQ(tempView.GetName(), toFind.GetName());
            EXPECT_STREQ(tempView.GetDescription(), toFind.GetDescription());
            }
        else if (entry.GetDgnViewId() == viewId2)
            {
            DgnViews::View toFind = viewTable.QueryView(viewId2);
            EXPECT_TRUE(tempView2.GetId() == toFind.GetId());
            EXPECT_TRUE(tempView2.GetBaseModelId() == toFind.GetBaseModelId());
            EXPECT_TRUE(tempView2.GetClassId() == toFind.GetClassId());
            EXPECT_TRUE(tempView2.GetDgnViewSource() == toFind.GetDgnViewSource());
            EXPECT_STREQ(tempView2.GetName(), toFind.GetName());
            EXPECT_STREQ(tempView2.GetDescription(), toFind.GetDescription());

            }
        }

    
    // Delete 
    //
    ASSERT_EQ (BE_SQLITE_DONE, viewTable.Delete(viewId)) << "Unable to delete the View";

    }

/*---------------------------------------------------------------------------------**//**
* Insert an Duplicate View
* @bsimethod                               Ahmed.Rizwan                    10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewsTest, InsertDuplicateView)
    {
    SetupProject (L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);

    // Get views
    DgnViews& viewTable = project->Views ();
    DgnViewId viewId ((uint64_t)4);
    // Create a new view
    DgnViews::View tempView = DgnViews::View (DgnViewType::Physical,
                                             DgnClassId(project->Schemas().GetECClassId("dgn","PhysicalView")),
                                             DgnModelId ((uint64_t)2),
                                             "TestView",
                                             0,
                                             DgnViewSource::User,
                                             DgnViewId ((uint64_t)5)
                                            );

    // Insert 
    ASSERT_EQ (BE_SQLITE_OK, viewTable.Insert(tempView)) << "Unable to insert View";
    tempView = viewTable.QueryView (DgnViewId ((uint64_t)5));
    EXPECT_TRUE (tempView.IsValid ()) << "View not found";
    EXPECT_STREQ ("TestView", tempView.GetName ()) << "View name does not match";
    // Verify second insertion fails
    ASSERT_EQ (BE_SQLITE_CONSTRAINT_UNIQUE, viewTable.Insert(tempView)) << "Second view should not be inserted";
    tempView = viewTable.QueryView (DgnViewId ((uint64_t)6));
    EXPECT_FALSE (tempView.IsValid ()) << "View should not be found";

    }
