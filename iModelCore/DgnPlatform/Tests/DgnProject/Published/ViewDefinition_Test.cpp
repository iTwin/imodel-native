/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ViewDefinition_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Shaun.Sewall                    02/2017
//----------------------------------------------------------------------------------------
struct ViewDefinitionTests : public DgnDbTestFixture
    {
    OrthographicViewDefinitionPtr InsertSpatialView(SpatialModelR model, Utf8CP name);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
OrthographicViewDefinitionPtr ViewDefinitionTests::InsertSpatialView(SpatialModelR model, Utf8CP name)
    {
    DgnDbR db = model.GetDgnDb();
    ModelSelectorPtr modelSelector = new ModelSelector(db, "");
    modelSelector->AddModel(model.GetModelId());

    OrthographicViewDefinitionPtr viewDef = new OrthographicViewDefinition(db, name, *new CategorySelector(db,""), *new DisplayStyle3d(db,""), *modelSelector);
    BeAssert(viewDef.IsValid());

    for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
        viewDef->GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());

    viewDef->SetStandardViewRotation(StandardView::Iso);
    viewDef->LookAtVolume(model.QueryModelRange());
    viewDef->Insert();
    BeAssert(viewDef->GetViewId().IsValid());
    return viewDef;
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
TEST_F(ViewDefinitionTests, MakeIterator)
    {
    SetupSeedProject();

    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(GetDgnDb(), "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "2");
    DrawingModelPtr model2 = DgnDbTestUtils::InsertDrawingModel(*drawing);

    DrawingViewDefinitionPtr view2A = DgnDbTestUtils::InsertDrawingView(*model2, "2A");
    DrawingViewDefinitionPtr view2B = DgnDbTestUtils::InsertDrawingView(*model2, "2B");
    DrawingViewDefinitionPtr view2C = DgnDbTestUtils::InsertDrawingView(*model2, "2C");
    ASSERT_FALSE(view2A->IsPrivate());
    ASSERT_FALSE(view2B->IsPrivate());
    ASSERT_FALSE(view2C->IsPrivate());

    view2A->SetIsPrivate(false);
    ASSERT_TRUE(view2A->Update().IsValid());
    ASSERT_FALSE(view2A->IsPrivate());
    ASSERT_FALSE(view2A->GetPropertyValueBoolean("IsPrivate"));

    view2B->SetIsPrivate(true);
    ASSERT_TRUE(view2B->Update().IsValid());
    ASSERT_TRUE(view2B->IsPrivate());
    ASSERT_TRUE(view2B->GetPropertyValueBoolean("IsPrivate"));

    PhysicalModelPtr model3 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "3");

    OrthographicViewDefinitionPtr view3A = InsertSpatialView(*model3, "3A");
    OrthographicViewDefinitionPtr view3B = InsertSpatialView(*model3, "3B");
    OrthographicViewDefinitionPtr view3C = InsertSpatialView(*model3, "3C");
    ASSERT_FALSE(view3A->IsPrivate());
    ASSERT_FALSE(view3B->IsPrivate());
    ASSERT_FALSE(view3C->IsPrivate());

    ASSERT_EQ(6, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(5, ViewDefinition::QueryCount(*m_db, "WHERE IsPrivate=FALSE"));
    ASSERT_EQ(1, ViewDefinition::QueryCount(*m_db, "WHERE IsPrivate=TRUE"));
    ASSERT_EQ(0, ViewDefinition::QueryCount(*m_db, "WHERE IsPrivate IS NULL"));

    int i=0;
    for (ViewDefinition::Entry const& iter : ViewDefinition::MakeIterator(*m_db, nullptr, "ORDER BY [CodeValue] DESC"))
        {
        switch (++i)
            {
            case 1:
                ASSERT_STREQ(iter.GetName(), "3C");
                ASSERT_FALSE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsOrthographicView());
                ASSERT_TRUE(iter.IsSpatialView());
                ASSERT_FALSE(iter.IsDrawingView());
                break;

            case 2:
                ASSERT_STREQ(iter.GetName(), "3B");
                ASSERT_FALSE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsOrthographicView());
                ASSERT_TRUE(iter.IsSpatialView());
                ASSERT_FALSE(iter.IsDrawingView());
                break;

            case 3:
                ASSERT_STREQ(iter.GetName(), "3A");
                ASSERT_FALSE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsOrthographicView());
                ASSERT_TRUE(iter.IsSpatialView());
                ASSERT_FALSE(iter.IsDrawingView());
                break;

            case 4:
                ASSERT_STREQ(iter.GetName(), "2C");
                ASSERT_FALSE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsDrawingView());
                ASSERT_FALSE(iter.IsSpatialView());
                break;

            case 5:
                ASSERT_STREQ(iter.GetName(), "2B");
                ASSERT_TRUE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsDrawingView());
                ASSERT_FALSE(iter.IsSpatialView());
                break;

            case 6:
                ASSERT_STREQ(iter.GetName(), "2A");
                ASSERT_FALSE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsDrawingView());
                ASSERT_FALSE(iter.IsSpatialView());
                break;

            default:
                ASSERT_TRUE(false);
                break;
            }
        }

    i=0;
    for (ViewDefinition::Entry const& iter : ViewDefinition::MakeIterator(*m_db, "WHERE IsPrivate=FALSE", "ORDER BY [CodeValue] ASC"))
        {
        switch (++i)
            {
            case 1: ASSERT_STREQ(iter.GetName(), "2A"); break;
            case 2: ASSERT_STREQ(iter.GetName(), "2C"); break;
            case 3: ASSERT_STREQ(iter.GetName(), "3A"); break;
            case 4: ASSERT_STREQ(iter.GetName(), "3B"); break;
            case 5: ASSERT_STREQ(iter.GetName(), "3C"); break;
            default:
                ASSERT_TRUE(false);
                break;
            }
        }

    for (ViewDefinition::Entry const& iter : ViewDefinition::MakeIterator(*m_db, "WHERE IsPrivate=TRUE"))
        {
        ASSERT_STREQ(iter.GetName(), "2B");
        }

    view3B->SetPropertyValue("IsPrivate", true);
    ASSERT_TRUE(view3B->Update().IsValid());
    ASSERT_TRUE(view3B->IsPrivate());
    ASSERT_TRUE(view3B->GetPropertyValueBoolean("IsPrivate"));
    ASSERT_EQ(2, ViewDefinition::QueryCount(*m_db, "WHERE IsPrivate=TRUE"));
    }
