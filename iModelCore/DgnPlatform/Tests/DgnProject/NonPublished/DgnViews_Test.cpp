/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnViews_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/DgnDbTestFixtures.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/ColorUtil.h>
#include <Bentley/bset.h>
#include <DgnPlatform/ViewDefinition.h>

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_SQLITE

#ifdef WIP_MERGE_Hassan

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnViewElemTest : public DgnDbTestFixture
{
    typedef ViewDefinition::Iterator Iter;
    typedef Iter::Options IterOpts;

    void SetupTestProject()
        {
        SetupSeedProject();

        DgnModelPtr model = AddModel("A");
        static const DgnViewSource s_viewSources[] = {DgnViewSource::User, DgnViewSource::Generated, DgnViewSource::Private};
        static const Utf8CP s_viewSourceNames[] = {"-U", "-G", "-P"};
        static const Utf8CP s_viewDescriptions[] = {"", "generated", "hidden"};
        // Create one new view of each source for each new model

        for (auto i = 0; i < _countof(s_viewSources); i++)
            {
            Utf8String viewName(model->GetCode().GetValue());
            viewName.append(s_viewSourceNames[i]);
            ViewDefinitionCPtr view = AddSpatialView<SpatialViewDefinition>(viewName, model->GetModelId(), s_viewSources[i], s_viewDescriptions[i]);
            ASSERT_TRUE(view.IsValid());
            ASSERT_TRUE(view->GetViewId().IsValid());
            }

        m_db->SaveChanges();
        }

    DgnModelPtr AddModel(Utf8StringCR name)
        {
        DgnClassId classId(m_db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialModel));
        DgnModel::CreateParams params(*m_db, classId, DgnModel::CreateModelCode(name));
        DgnModelPtr model = new SpatialModel(params);
        EXPECT_EQ(DgnDbStatus::Success, model->Insert());

        return model;
        }
        
    template<typename T> ViewDefinitionCPtr AddSpatialView(Utf8StringCR name, DgnModelId baseModelId, DgnViewSource source, Utf8StringCR descr="")
        {
        T view(*m_db, name, *new CategorySelector(*m_db, ""), *new DisplayStyle3d(*m_db, ""), *new ModelSelector(*m_db, ""));
        view.SetDescr(descr);
        view.SetSource(source);
        view.GetModelSelector().AddModel(baseModelId);
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
        Iter iter(*m_db, opts);
        ExpectViews(iter, names);
        }

    void Make4Views()
        {
        PhysicalModelPtr m1 = DgnDbTestUtils::InsertPhysicalModel(GetDgnDb(), "m1");
        PhysicalModelPtr m2 = DgnDbTestUtils::InsertPhysicalModel(GetDgnDb(), "m2");
        PhysicalModelPtr m3 = DgnDbTestUtils::InsertPhysicalModel(GetDgnDb(), "m3");
        PhysicalModelPtr m4 = DgnDbTestUtils::InsertPhysicalModel(GetDgnDb(), "m4");

        ASSERT_TRUE(DgnDbTestUtils::InsertCameraView(*m1, "View 1").IsValid());
        ASSERT_TRUE(DgnDbTestUtils::InsertCameraView(*m2, "View 2").IsValid());
        ASSERT_TRUE(DgnDbTestUtils::InsertCameraView(*m3, "View 3").IsValid());
        ASSERT_TRUE(DgnDbTestUtils::InsertCameraView(*m4, "View 4").IsValid());
        }
};

/*---------------------------------------------------------------------------------**//**
* Work with Views
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewElemTest, WorkWithViewTable)
    {
    SetupSeedProject();
    Make4Views();

    //Get views
    auto iter = ViewDefinition::MakeIterator(*m_db);
    EXPECT_EQ(3, ViewDefinition::QueryCount(*m_db));

    //Iterate through each view and make sure they have correct information
    static const Utf8CP s_viewNames[] = { "A-U", "A-G", "A-P"};

    int i = 0;
    for (auto const& entry : iter)
        {
        EXPECT_STREQ(entry.GetName(), s_viewNames[i]);
        i++;
        }

    ASSERT_EQ(i, 3);
    }

/*---------------------------------------------------------------------------------**//**
* Delete view test
* @bsimethod                               Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewElemTest, DeleteView)
    {
    SetupSeedProject();
    Make4Views();

    //Get views
    auto viewId = (*ViewDefinition::MakeIterator(*m_db).begin()).GetId();
    auto view = ViewDefinition::QueryView(viewId, *m_db);
    ASSERT_TRUE(view.IsValid());

    EXPECT_EQ(DgnDbStatus::Success, view->Delete());
    
    view = ViewDefinition::QueryView(viewId, *m_db);
    EXPECT_FALSE(view.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* Set name test view
* @bsimethod                               Algirdas.Mikoliunas                   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewElemTest, SetViewName)
    {
    SetupSeedProject();
    Make4Views();

    //Get views
    auto viewId = (*ViewDefinition::MakeIterator(*m_db).begin()).GetId();
    auto cpView = ViewDefinition::QueryView(viewId, *m_db);
    ASSERT_TRUE(cpView.IsValid());
    auto view = cpView->MakeCopy<ViewDefinition>();
    ASSERT_TRUE(view.IsValid());
    
    EXPECT_STRNE("TestView", view->GetName().c_str());
    EXPECT_EQ(DgnDbStatus::Success, view->SetName("TestView2"));
    EXPECT_STREQ("TestView2", view->GetName().c_str());
    
    EXPECT_TRUE(view->Update().IsValid());
    
    cpView = ViewDefinition::QueryView(viewId, *m_db);
    ASSERT_TRUE(view.IsValid());
    EXPECT_STREQ("TestView2", view->GetName().c_str());
    }

#if defined (NEEDS_WORK_TARGET_MODEL)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Umar Hayat                    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewElemTest, CRUD)
    {
    SetupSeedProject();

    Utf8CP drawingName = "TestDrawing";
    Utf8CP drawingDescr = "TestDrawing descr";

    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, DgnModel::CreateModelCode("DrawingListModel"));
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, drawingName);
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing, "TestDrawingModel");
    DrawingViewPtr drawingView = DgnDbTestUtils::InsertDrawingView(*drawingModel, drawingDescr);

    PhysicalModelPtr physicalModel = InsertPhysicalModel("TestPhys");
    DgnViewId cameraViewId = InsertCameraView(*physicalModel, cameraViewName);

    // Create a new view
    SpatialViewDefinition tempView(*m_db, "TestView", );
    tempView.SetDescr("Test Description");
    tempView.SetModelSelector(*DgnDbTestUtils::InsertModelSelector(*m_db, "TestView", DgnModel::DictionaryId()));

    DrawingViewDefinition tempView2(*m_db, "TestDrawingView", DgnModelId((uint64_t)1)); // FIXME: Need to point at a DrawingModel!
    tempView2.SetDescr("TestDrawingView Description");

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
    EXPECT_TRUE( viewId == ViewDefinition::QueryViewId(ViewDefinition::CreateCode("TestView"), *m_db));

    //  Iterate
    //
    for (auto const& entry : ViewDefinition::MakeIterator(*m_db))
        {
        auto toFind = ViewDefinition::QueryView(entry.GetId(), *m_db);
        ASSERT_TRUE(toFind.IsValid());
        if (entry.GetId() == viewId)
            {
            EXPECT_TRUE(tempView.GetViewId() == toFind->GetViewId());
            EXPECT_TRUE(tempView.GetElementClassId() == toFind->GetElementClassId());
            EXPECT_TRUE(tempView.GetSource() == toFind->GetSource());
            EXPECT_STREQ(tempView.GetName().c_str(), toFind->GetName().c_str());
            EXPECT_STREQ(tempView.GetDescr().c_str(), toFind->GetDescr().c_str());
            }
        else if (entry.GetId() == viewId2)
            {
            EXPECT_TRUE(tempView2.GetViewId() == toFind->GetViewId());
            auto dview = m_db->Elements().Get<DrawingViewDefinition>(tempView2.GetViewId());
            auto dviewToFind = m_db->Elements().Get<DrawingViewDefinition>(toFind->GetViewId());
            EXPECT_TRUE(dview->GetBaseModelId() == dviewToFind->GetBaseModelId());
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnViewElemTest, Iterate)
    {
    SetupSeedProject();

    PhysicalModelPtr models[] = { DgnDbTestUtils::InsertPhysicalModel(*m_db, "A"), DgnDbTestUtils::InsertPhysicalModel(*m_db, "B") };
    static const DgnViewSource s_viewSources[] = { DgnViewSource::User, DgnViewSource::Generated, DgnViewSource::Private };
    static const Utf8CP s_viewSourceNames[] = { "-U", "-G", "-P" };
    static const Utf8CP s_viewDescriptions[] = { "", "generated", "hidden" };

    // Delete all existing views
    for (auto const& entry : ViewDefinition::MakeIterator(*m_db))
        {
        auto view = ViewDefinition::QueryView(entry.GetId(), *m_db);
        ASSERT_TRUE(view.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, view->Delete());
        }

    ASSERT_EQ(0, ViewDefinition::QueryCount(*m_db));

    // Create one new view of each source for each new model
    for (auto const& model : models)
        {
        for (auto i = 0; i < _countof(s_viewSources); i++)
            {
            Utf8String viewName(model->GetName());
            viewName.append(s_viewSourceNames[i]);
            ViewDefinitionCPtr view = AddSpatialView<SpatialViewDefinition>(viewName, model->GetModelId(), s_viewSources[i], s_viewDescriptions[i]);
            ASSERT_TRUE(view.IsValid());
            ASSERT_TRUE(view->GetViewId().IsValid());
            }
        }

    size_t nExpectedViews = _countof(models) * _countof(s_viewSources);
    EXPECT_EQ(nExpectedViews, ViewDefinition::QueryCount(*m_db));

    // All
    ExpectViews(IterOpts(), { "A-U", "A-G", "A-P", "B-U", "B-G", "B-P" });

    // Ordering
    ExpectViews(IterOpts(IterOpts::Source::All, IterOpts::Order::Unordered), { "A-U", "A-G", "A-P", "B-U", "B-G", "B-P" });
    ExpectViews(IterOpts(IterOpts::Source::All, IterOpts::Order::Ascending), { "A-G", "A-P", "A-U", "B-G", "B-P", "B-U" });

#ifdef WIP_VIEW_DEFINITION
    // Base model
    ExpectViews(IterOpts(models[0]->GetModelId()), { "A-U", "A-G", "A-P" });
    ExpectViews(IterOpts(models[1]->GetModelId()), { "B-U", "B-G", "B-P" });
    ExpectViews(IterOpts(models[1]->GetModelId(), IterOpts::Order::Ascending), { "B-G", "B-P", "B-U" });
#endif

    // Source
    ExpectViews(IterOpts(IterOpts::Source::User), { "A-U", "B-U" });
    ExpectViews(IterOpts(IterOpts::Source::Generated | IterOpts::Source::Private), { "A-G", "A-P", "B-G", "B-P" });

    // Combo
#ifdef WIP_VIEW_DEFINITION
    ExpectViews(IterOpts(IterOpts::Source::Generated | IterOpts::Source::User, IterOpts::Order::Ascending, models[0]->GetModelId()), { "A-G", "A-U" });
#endif

    // Custom
    ExpectViews(IterOpts("WHERE Descr='generated' ORDER BY [CodeValue] DESC"), { "B-G", "A-G" });

    // Deleting a model deletes all views for which it is the *only* model in the ModelSelector
    EXPECT_EQ(DgnDbStatus::Success, models[0]->Delete());
    EXPECT_EQ(_countof(s_viewSources), ViewDefinition::QueryCount(*m_db));
    ExpectViews(IterOpts(), { "B-U", "B-G", "B-P" });
    }

#endif
