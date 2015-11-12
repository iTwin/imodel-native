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
#include <DgnPlatform/DgnView.h>

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

        void SetTestViewProperties (WString Name)
            {
            tvName = Name;
            };
        void IsEqual (TestViewProperties testView)
            {
            EXPECT_STREQ (tvName.c_str(), testView.tvName.c_str()) << "Names don't match";
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
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnViewElemTest : DgnViewsTest
{
    void SetupProject()
        {
        DgnViewsTest::SetupProject(L"ElementsSymbologyByLevel.idgndb", Db::OpenMode::ReadWrite);
        }
};

/*---------------------------------------------------------------------------------**//**
* Work with Views
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewElemTest, WorkWithViewTable)
    {
    SetupProject();

    //Get views
    auto iter = ViewDefinition::MakeIterator(*project);
    //ASSERT_EQ (4, iter.QueryCount()) <<"The expected view count is 4 where as it is: " << iter.QueryCount();

    //Iterate through each view and make sure they have correct information
    TestViewProperties fileViews[4], testView;
    fileViews[0].SetTestViewProperties (L"Default - View 1");
    fileViews[1].SetTestViewProperties (L"Default - View 2");
    fileViews[2].SetTestViewProperties (L"Model2d Views - View 1");
    fileViews[3].SetTestViewProperties (L"Model2d Views - View 2");

    int i = 0;
    for (auto const& entry : iter)
        {
        WString entryNameW (entry.GetName(), true);
        testView.SetTestViewProperties (entryNameW.c_str());
        testView.IsEqual (fileViews[i]);
        i++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Delete view test
* @bsimethod                               Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewElemTest, DeleteView)
    {
    SetupProject();

    //Get views
    auto viewId = (*ViewDefinition::MakeIterator(*project).begin()).GetId();
    auto view = ViewDefinition::QueryView(viewId, *project);
    ASSERT_TRUE(view.IsValid());

    EXPECT_EQ(DgnDbStatus::Success, view->Delete());
    
    view = ViewDefinition::QueryView(viewId, *project);
    EXPECT_FALSE(view.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* Set name test view
* @bsimethod                               Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewElemTest, SetViewName)
    {
    SetupProject();

    //Get views
    auto viewId = (*ViewDefinition::MakeIterator(*project).begin()).GetId();
    auto cpView = ViewDefinition::QueryView(viewId, *project);
    ASSERT_TRUE(cpView.IsValid());
    auto view = cpView->MakeCopy<ViewDefinition>();
    ASSERT_TRUE(view.IsValid());
    
    EXPECT_STRNE("TestView", view->GetName().c_str());
    EXPECT_EQ(DgnDbStatus::Success, view->SetName("TestView"));
    EXPECT_STREQ("TestView", view->GetName().c_str());
    
    EXPECT_TRUE(view->Update().IsValid());
    
    cpView = ViewDefinition::QueryView(viewId, *project);
    ASSERT_TRUE(view.IsValid());
    EXPECT_STREQ("TestView", view->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* CRUD
* @bsimethod                               Umar Hayagt                    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewElemTest, CRUD)
    {
    SetupProject();

    // Create a new view
    CameraViewDefinition tempView(CameraViewDefinition::CreateParams(*project, "TestView",
                                  CameraViewDefinition::Data(DgnModelId((uint64_t)2), DgnViewSource::User, "Test Description")));
    DrawingViewDefinition tempView2(DrawingViewDefinition::CreateParams(*project, "TestDrawingView",
                                    DrawingViewDefinition::Data(DgnModelId((uint64_t)1), DgnViewSource::Private, "TestDrawingView Description")));

    // Insert 
    auto cpView = tempView.Insert();
    ASSERT_TRUE(cpView.IsValid());
    EXPECT_EQ(tempView.GetViewId(), cpView->GetViewId());
    EXPECT_TRUE(tempView.GetViewId().IsValid());

    auto cpView2 = tempView2.Insert();
    ASSERT_TRUE(cpView2.IsValid());
    EXPECT_EQ(cpView2->GetViewId(), tempView2.GetViewId());
    EXPECT_TRUE(tempView2.GetViewId().IsValid());

    //  Query
    //
    auto viewId = tempView.GetViewId();
    auto viewId2 = tempView2.GetViewId();
    EXPECT_TRUE( viewId == ViewDefinition::QueryViewId(ViewDefinition::CreateCode("TestView"), *project));

    //  Iterate
    //
    for (auto const& entry : ViewDefinition::MakeIterator(*project))
        {
        auto toFind = ViewDefinition::QueryView(entry.GetId(), *project);
        ASSERT_TRUE(toFind.IsValid());
        if (entry.GetId() == viewId)
            {
            EXPECT_TRUE(tempView.GetViewId() == toFind->GetViewId());
            EXPECT_TRUE(tempView.GetBaseModelId() == toFind->GetBaseModelId());
            EXPECT_TRUE(tempView.GetElementClassId() == toFind->GetElementClassId());
            EXPECT_TRUE(tempView.GetSource() == toFind->GetSource());
            EXPECT_STREQ(tempView.GetName().c_str(), toFind->GetName().c_str());
            EXPECT_STREQ(tempView.GetDescr().c_str(), toFind->GetDescr().c_str());
            }
        else if (entry.GetId() == viewId2)
            {
            EXPECT_TRUE(tempView2.GetViewId() == toFind->GetViewId());
            EXPECT_TRUE(tempView2.GetBaseModelId() == toFind->GetBaseModelId());
            EXPECT_TRUE(tempView2.GetElementClassId() == toFind->GetElementClassId());
            EXPECT_TRUE(tempView2.GetSource() == toFind->GetSource());
            EXPECT_STREQ(tempView2.GetName().c_str(), toFind->GetName().c_str());
            EXPECT_STREQ(tempView2.GetDescr().c_str(), toFind->GetDescr().c_str());
            }
        }

    // Delete 
    //
    EXPECT_EQ(DgnDbStatus::Success, cpView->Delete());
    }


