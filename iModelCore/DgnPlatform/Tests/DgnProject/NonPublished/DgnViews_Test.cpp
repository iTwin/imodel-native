/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnViews_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnViews
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnViewsTest : public ::testing::Test
    {
public:
    ScopedDgnHost   m_host;
    DgnDbPtr        project;

    void SetupProject (WCharCP projFile, Db::OpenMode mode);
    };

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .bim project file
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewsTest::SetupProject (WCharCP projFile, Db::OpenMode mode)
    {
    DgnDbTestDgnManager tdm (projFile, __FILE__, mode, false);
    project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnViewElemTest : DgnViewsTest
{
    typedef ViewDefinition::Iterator Iter;
    typedef Iter::Options IterOpts;

    void SetupProject()
        {
        DgnViewsTest::SetupProject(L"ElementsSymbologyByLevel.ibim", Db::OpenMode::ReadWrite);
        }

    DgnModelPtr AddModel(Utf8StringCR name)
        {
        DgnClassId classId(project->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialModel));
        DgnModel::CreateParams params(*project, classId, DgnModel::CreateModelCode(name));
        DgnModelPtr model = new SpatialModel(params);
        EXPECT_EQ(DgnDbStatus::Success, model->Insert());

        return model;
        }

    template<typename T> ViewDefinitionCPtr AddView(Utf8StringCR name, DgnModelId baseModelId, DgnViewSource source, Utf8StringCR descr="")
        {
        typename T::CreateParams params(*project, name, ViewDefinition::Data(baseModelId, source, descr));
        T view(params);
        auto cpView = view.Insert();
        EXPECT_TRUE(cpView.IsValid());
        return cpView;
        }

    void ExpectViews(Iter& iter, std::initializer_list<Utf8CP> names)
        {
        auto nameIter = std::begin(names);
        for (auto const& entry : iter)
            {
            EXPECT_FALSE(std::end(names) == nameIter);
            if (std::end(names) == nameIter)
                break;

            EXPECT_STREQ(entry.GetName(), *nameIter);
            ++nameIter;
            }

        EXPECT_TRUE(nameIter == std::end(names));
        }

    void ExpectViews(IterOpts const& opts, std::initializer_list<Utf8CP> names)
        {
        Iter iter(*project, opts);
        ExpectViews(iter, names);
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
    EXPECT_EQ(4, ViewDefinition::QueryCount(*project));

    //Iterate through each view and make sure they have correct information
    static const Utf8CP s_viewNames[] = { "Default - View 1", "Default - View 2", "Model2d Views - View 1", "Model2d Views - View 2" };

    int i = 0;
    for (auto const& entry : iter)
        {
        EXPECT_STREQ(entry.GetName(), s_viewNames[i]);
        i++;
        }

    EXPECT_EQ(i, 4);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewElemTest, Iterate)
    {
    SetupProject();

    DgnModelPtr models[] = { AddModel("A"), AddModel("B") };
    static const DgnViewSource s_viewSources[] = { DgnViewSource::User, DgnViewSource::Generated, DgnViewSource::Private };
    static const Utf8CP s_viewSourceNames[] = { "-U", "-G", "-P" };
    static const Utf8CP s_viewDescriptions[] = { "", "generated", "hidden" };

    // Delete all existing views
    for (auto const& entry : ViewDefinition::MakeIterator(*project))
        {
        auto view = ViewDefinition::QueryView(entry.GetId(), *project);
        ASSERT_TRUE(view.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, view->Delete());
        }

    ASSERT_EQ(0, ViewDefinition::QueryCount(*project));

    // Create one new view of each source for each new model
    for (auto const& model : models)
        {
        for (auto i = 0; i < _countof(s_viewSources); i++)
            {
            Utf8String viewName(model->GetCode().GetValue());
            viewName.append(s_viewSourceNames[i]);
            ViewDefinitionCPtr view = AddView<SpatialViewDefinition>(viewName, model->GetModelId(), s_viewSources[i], s_viewDescriptions[i]);
            ASSERT_TRUE(view.IsValid());
            ASSERT_TRUE(view->GetViewId().IsValid());
            }
        }

    size_t nExpectedViews = _countof(models) * _countof(s_viewSources);
    EXPECT_EQ(nExpectedViews, ViewDefinition::QueryCount(*project));

    // All
    ExpectViews(IterOpts(), { "A-U", "A-G", "A-P", "B-U", "B-G", "B-P" });

    // Ordering
    ExpectViews(IterOpts(IterOpts::Source::All, IterOpts::Order::Unordered), { "A-U", "A-G", "A-P", "B-U", "B-G", "B-P" });
    ExpectViews(IterOpts(IterOpts::Source::All, IterOpts::Order::Ascending), { "A-G", "A-P", "A-U", "B-G", "B-P", "B-U" });

    // Base model
    ExpectViews(IterOpts(models[0]->GetModelId()), { "A-U", "A-G", "A-P" });
    ExpectViews(IterOpts(models[1]->GetModelId()), { "B-U", "B-G", "B-P" });
    ExpectViews(IterOpts(models[1]->GetModelId(), IterOpts::Order::Ascending), { "B-G", "B-P", "B-U" });

    // Source
    ExpectViews(IterOpts(IterOpts::Source::User), { "A-U", "B-U" });
    ExpectViews(IterOpts(IterOpts::Source::Generated | IterOpts::Source::Private), { "A-G", "A-P", "B-G", "B-P" });

    // Combo
    ExpectViews(IterOpts(IterOpts::Source::Generated | IterOpts::Source::User, IterOpts::Order::Ascending, models[0]->GetModelId()), { "A-G", "A-U" });

    // Custom
    ExpectViews(IterOpts("WHERE Descr='generated' ORDER BY Code.[Value] DESC"), { "B-G", "A-G" });

    // Deleting a model deletes all views which use it as a base model
    EXPECT_EQ(DgnDbStatus::Success, models[0]->Delete());
    EXPECT_EQ(_countof(s_viewSources), ViewDefinition::QueryCount(*project));
    ExpectViews(IterOpts(), { "B-U", "B-G", "B-P" });
    }

