/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/ViewTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

//----------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald                07/18
//----------------------------------------------------------------------------------------
struct ViewTests : public ConverterTestBaseFixture
{
    DEFINE_T_SUPER(ConverterTestBaseFixture);

protected:
    DgnV8Api::NamedViewPtr CreateAndWriteSavedView(Bentley::DgnFileR dgnFile, WCharCP viewName, int view);
    int GetViewCount(DgnDbR db);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
DgnV8Api::NamedViewPtr ViewTests::CreateAndWriteSavedView(Bentley::DgnFileR dgnFile, WCharCP viewName, int view)
    {
    DgnV8Api::NamedViewPtr namedView;

    EXPECT_TRUE(DgnV8Api::NamedViewStatus::Success == DgnV8Api::NamedView::Create(namedView, dgnFile, viewName));
    EXPECT_EQ(true, namedView.IsValid());
    DgnV8Api::ModelId mId = dgnFile.FindModelIdByName(L"Default");
    DgnV8Api::ViewGroupPtr viewGroup = dgnFile.GetViewGroupsR().FindByModelId(mId, true, -1);

    DgnV8ViewInfoR viewInfo = viewGroup->GetViewInfoR(view);
    namedView->SetViewInfo(viewInfo);

    EXPECT_TRUE(DgnV8Api::NamedViewStatus::Success == namedView->WriteToFile());
    return namedView;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
int ViewTests::GetViewCount(DgnDbR db)
    {
    auto iter = ViewDefinition::MakeIterator(db);
    int foundViewCount = 0;
    for (auto const& entry : iter)
        {
        foundViewCount++;
        }
    return foundViewCount;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ViewTests, SavedViewsCRUD)
    {
    LineUpFiles(L"CreateViews.ibim", L"Test3d.dgn", false);
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    DgnV8Api::NamedViewPtr view1 = CreateAndWriteSavedView(*v8editor.m_file, L"View1", 0);
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    DgnViewId view1Id;
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_EQ(2, GetViewCount(*db)); // Default view + saved view
        DefinitionModelPtr definitionModel = GetJobDefinitionModel(*db);
        view1Id = ViewDefinition::QueryViewId(*definitionModel, "View1");
        ASSERT_TRUE(view1Id.IsValid());
        }

    // Confirm that saved view created after initial conversion gets converted during update
    DgnV8Api::NamedViewPtr view2 = CreateAndWriteSavedView(*v8editor.m_file, L"View2", 0);
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    DgnViewId view2Id;
    DgnElementId modelSelectorId;
    DgnElementId categorySelectorId;
    DgnElementId displayStyleId;
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_EQ(3, GetViewCount(*db)); // default view + 2 saved views
        DefinitionModelPtr definitionModel = GetJobDefinitionModel(*db);
        view2Id = ViewDefinition::QueryViewId(*definitionModel, "View2");
        ASSERT_TRUE(view2Id.IsValid());
        SpatialViewDefinitionCPtr tempView = db->Elements().Get<SpatialViewDefinition>(view2Id);
        modelSelectorId = tempView->GetModelSelectorId();
        categorySelectorId = tempView->GetCategorySelectorId();
        displayStyleId = tempView->GetDisplayStyleId();
        }

    // Delete view
    ASSERT_EQ(DgnV8Api::NamedViewStatus::Success, view2->DeleteFromFile());
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_EQ(2, GetViewCount(*db)); // default view + 1 saved views
        DefinitionModelPtr definitionModel = GetJobDefinitionModel(*db);
        auto viewId = ViewDefinition::QueryViewId(*definitionModel, "View2");
        ASSERT_FALSE(viewId.IsValid());
        auto cat = db->Elements().GetElement(categorySelectorId);
        ASSERT_FALSE(cat.IsValid());
        auto display = db->Elements().GetElement(displayStyleId);
        ASSERT_FALSE(display.IsValid());
        auto model = db->Elements().GetElement(modelSelectorId);
        ASSERT_FALSE(model.IsValid());
        }

    // Updates
    EXPECT_TRUE(DgnV8Api::NamedViewStatus::Success == view1->SetName(L"New Name"));
    EXPECT_TRUE(DgnV8Api::NamedViewStatus::Success == view1->WriteToFile());
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_EQ(2, GetViewCount(*db)); // default view + 1 saved views
        DefinitionModelPtr definitionModel = GetJobDefinitionModel(*db);
        auto viewId = ViewDefinition::QueryViewId(*definitionModel, "View1");
        ASSERT_FALSE(viewId.IsValid());
        viewId = ViewDefinition::QueryViewId(*definitionModel, "New Name");
        ASSERT_TRUE(viewId.IsValid());

        ASSERT_TRUE(viewId == view1Id);
        }

    BentleyApi::BeFileName secondV8File = GetOutputFileName(L"SecondV8.dgn");
    BentleyApi::BeFileName seedFile = GetInputFileName(L"Test3d.dgn");
    ASSERT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::BeCopyFile(seedFile, secondV8File)) << "Unable to copy file \nSource: [" << Utf8String(seedFile.c_str()).c_str() << "]\nDestination: [" << Utf8String(secondV8File
                                                                                                                                                                                                                                   .c_str()).c_str() << "]";
    V8FileEditor v8editor2;
    v8editor2.Open(secondV8File);
    DgnV8Api::NamedViewPtr view13 = CreateAndWriteSavedView(*v8editor2.m_file, L"View 3", 0);
    DoConvert(m_dgnDbFileName, secondV8File);
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_EQ(4, GetViewCount(*db)); // default view + 1 saved views for each file
        auto temp = db->Elements().GetElement(view1Id);
        ASSERT_TRUE(temp.IsValid()); // Ensure the view from the first file still exists

        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ViewTests, SavedViewsInDrawings)
    { }

