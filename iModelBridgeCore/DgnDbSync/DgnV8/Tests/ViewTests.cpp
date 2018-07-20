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

        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_EQ(2, GetViewCount(*db)); // Default view + saved view
        DefinitionModelPtr definitionModel = GetJobDefinitionModel(*db);
        auto viewId = ViewDefinition::QueryViewId(*definitionModel, "View1");
        ASSERT_TRUE(viewId.IsValid());
        }

    // Confirm that saved view created after initial conversion gets converted during update
    DgnV8Api::NamedViewPtr view2 = CreateAndWriteSavedView(*v8editor.m_file, L"View2", 0);
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    DgnViewId view2Id;
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_EQ(3, GetViewCount(*db)); // default view + 2 saved views
        DefinitionModelPtr definitionModel = GetJobDefinitionModel(*db);
        view2Id = ViewDefinition::QueryViewId(*definitionModel, "View2");
        ASSERT_TRUE(view2Id.IsValid());
        }

    ASSERT_EQ(DgnV8Api::NamedViewStatus::Success, view2->DeleteFromFile());
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        ASSERT_EQ(2, GetViewCount(*db)); // default view + 1 saved views
        DefinitionModelPtr definitionModel = GetJobDefinitionModel(*db);
        auto viewId = ViewDefinition::QueryViewId(*definitionModel, "View2");
        ASSERT_FALSE(viewId.IsValid());
        }

    }

