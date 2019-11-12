/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//========================================================================================
// @bsiclass                                    Shaun.Sewall                    02/2017
//========================================================================================
struct ViewDefinitionTests : public DgnDbTestFixture
{
    OrthographicViewDefinitionPtr InsertSpatialView(SpatialModelR model, Utf8CP name, bool isPrivate);
    SheetViewDefinitionCPtr InsertSheetView(Sheet::ModelR model, Utf8CP name, bool isPrivate);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
OrthographicViewDefinitionPtr ViewDefinitionTests::InsertSpatialView(SpatialModelR model, Utf8CP name, bool isPrivate)
    {
    DgnDbR db = model.GetDgnDb();
    DefinitionModelR dictionary = db.GetDictionaryModel();
    ModelSelectorPtr modelSelector = new ModelSelector(dictionary, "");
    modelSelector->AddModel(model.GetModelId());

    OrthographicViewDefinitionPtr viewDef = new OrthographicViewDefinition(dictionary, name, *new CategorySelector(dictionary, ""), *new DisplayStyle3d(dictionary, ""), *modelSelector);
    BeAssert(viewDef.IsValid());

    for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
        viewDef->GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());

    viewDef->SetIsPrivate(isPrivate);
    viewDef->SetStandardViewRotation(StandardView::Iso);
    viewDef->LookAtVolume(model.QueryElementsRange());
    viewDef->Insert();
    BeAssert(viewDef->GetViewId().IsValid());
    return viewDef;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
SheetViewDefinitionCPtr ViewDefinitionTests::InsertSheetView(Sheet::ModelR model, Utf8CP name, bool isPrivate)
    {
    DgnDbR db = model.GetDgnDb();
    DefinitionModelR dictionary = db.GetDictionaryModel();

    SheetViewDefinition sheetView(dictionary, name, model.GetModelId(), *new CategorySelector(dictionary, ""), *new DisplayStyle2d(dictionary, ""));
    sheetView.SetIsPrivate(isPrivate);
    SheetViewDefinitionCPtr result = db.Elements().Insert<SheetViewDefinition>(sheetView);
    BeAssert(result.IsValid());
    return result;
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

    OrthographicViewDefinitionPtr view3A = InsertSpatialView(*model3, "3A", false);
    OrthographicViewDefinitionPtr view3B = InsertSpatialView(*model3, "3B", false);
    OrthographicViewDefinitionPtr view3C = InsertSpatialView(*model3, "3C", false);
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

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
TEST_F(ViewDefinitionTests, QueryDefaultViewId)
    {
    SetupSeedProject();
    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "PhysicalModel");
    DocumentListModelPtr documentListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DocumentList");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*documentListModel, "Drawing");
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    Sheet::ElementPtr sheet = DgnDbTestUtils::InsertSheet(*documentListModel, 1.0, 1.0, 1.0, "Sheet");
    Sheet::ModelPtr sheetModel = DgnDbTestUtils::InsertSheetModel(*sheet);

    ASSERT_EQ(0, ViewDefinition::QueryCount(*m_db));
    ASSERT_FALSE(ViewDefinition::QueryDefaultViewId(*m_db).IsValid());
    ASSERT_EQ(BentleyStatus::SUCCESS, m_db->Schemas().CreateClassViewsInDb());

    DrawingViewDefinitionPtr privateDrawingView = DgnDbTestUtils::InsertDrawingView(*drawingModel, "PrivateDrawingView");
    privateDrawingView->SetIsPrivate(true);
    ASSERT_TRUE(privateDrawingView->Update().IsValid());
    ASSERT_EQ(1, ViewDefinition::QueryCount(*m_db));
    ASSERT_FALSE(ViewDefinition::QueryDefaultViewId(*m_db).IsValid()); // private DrawingViews not considered

    DrawingViewDefinitionPtr publicDrawingView = DgnDbTestUtils::InsertDrawingView(*drawingModel, "PublicDrawingView");
    ASSERT_EQ(2, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicDrawingView->GetViewId());

    InsertSheetView(*sheetModel, "PrivateSheetView", true);
    ASSERT_EQ(3, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicDrawingView->GetViewId()); // private SheetViews not considered

    SheetViewDefinitionCPtr publicSheetView = InsertSheetView(*sheetModel, "PublicSheetView", false);
    ASSERT_EQ(4, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicSheetView->GetViewId()); // public SheetViews take precendence over public DrawingViews

    OrthographicViewDefinitionPtr privateSpatialView = InsertSpatialView(*physicalModel, "PrivateSpatialView", true);
    ASSERT_EQ(5, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicSheetView->GetViewId()); // private SpatialViews not considered

    OrthographicViewDefinitionPtr publicSpatialView = InsertSpatialView(*physicalModel, "PublicSpatialView", false);
    ASSERT_EQ(6, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicSpatialView->GetViewId()); // public SpatialViews take precendence over public SheetViews

    DgnViewId defaultViewId(m_db->Elements().GetRootSubjectId().GetValue()); // not a valid view
    m_db->SaveProperty(DgnViewProperty::DefaultView(), &defaultViewId, sizeof(defaultViewId));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicSpatialView->GetViewId()); // invalid default view property should be ignored

    defaultViewId = privateSpatialView->GetViewId();
    m_db->SaveProperty(DgnViewProperty::DefaultView(), &defaultViewId, sizeof(defaultViewId));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), privateSpatialView->GetViewId()); // valid default view property should take precedence
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                               Ridha.Malik                   3/17
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ViewDefinitionTests, ViewDefinition2dCRUD)
    {
    SetupSeedProject();
    DgnViewId DviewDefid, SviewDefid,RviewDefid;
    if (true)
        {
        DefinitionModelR dictionary = m_db->GetDictionaryModel();
        DocumentListModelPtr DocListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DrawingListModel");
        DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*DocListModel, "TestDrawingModel");
        DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);

        CategorySelectorPtr categories = new CategorySelector(dictionary, "");
        for (ElementIteratorEntryCR categoryEntry : DrawingCategory::MakeIterator(*m_db))
            categories->AddCategory(categoryEntry.GetId<DgnCategoryId>());

        DisplayStyle2dPtr style = new DisplayStyle2d(dictionary, "");
        ASSERT_TRUE(style.IsValid());
        Render::ViewFlags flags = style->GetViewFlags();
        flags.SetRenderMode(Render::RenderMode::SmoothShade);
        style->SetViewFlags(flags);
        //Create a  DrawingView
        DrawingViewDefinitionPtr viewDef = new DrawingViewDefinition(dictionary, "DrawingView", drawingModel->GetModelId(), *categories, *style);
        ASSERT_TRUE(viewDef.IsValid());
        ASSERT_EQ(viewDef->GetName(), "DrawingView");
        viewDef->SetDescription("DrawingView Descr");
        ASSERT_EQ(viewDef->GetDescription(), "DrawingView Descr");
        ASSERT_TRUE(viewDef->IsDrawingView());
        viewDef->SetIsPrivate(false);
        ASSERT_FALSE(viewDef->IsPrivate());
        //Insert DrawingView
        ViewDefinitionCPtr viewDefele = viewDef->Insert();
        ASSERT_TRUE(viewDefele.IsValid());
        DviewDefid = viewDefele->GetViewId();
        ASSERT_TRUE(DviewDefid == ViewDefinition::QueryViewId(dictionary, "DrawingView"));
        //Create SheetView
        Sheet::ElementPtr sheet = DgnDbTestUtils::InsertSheet(*DocListModel, 1.0, 1.0, 1.0, "MySheet");
        Sheet::ModelPtr sheetModel = DgnDbTestUtils::InsertSheetModel(*sheet);
        SheetViewDefinitionPtr sheetView=new SheetViewDefinition(dictionary, "MySheetView", sheetModel->GetModelId(), *categories, *style);
        ASSERT_TRUE(sheetView.IsValid());
        ASSERT_EQ(sheetView->GetName(), "MySheetView");
        sheetView->SetDescription("SheetView Descr");
        ASSERT_EQ(sheetView->GetDescription(), "SheetView Descr");
        ASSERT_TRUE(sheetView->IsSheetView());
        sheetView->SetIsPrivate(false);
        ASSERT_FALSE(sheetView->IsPrivate());
        //Insert SheetView
        ViewDefinitionCPtr sheetviewele=sheetView->Insert();
        SviewDefid = sheetviewele->GetViewId();
        ASSERT_TRUE(SviewDefid == ViewDefinition::QueryViewId(dictionary, "MySheetView"));
        }
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    //Check what stored in Db and then Update DrawingViewDefinition
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    //DrawingViewDefinition
    DrawingViewDefinitionPtr viewDef = m_db->Elements().GetForEdit<DrawingViewDefinition>(DviewDefid);
    ASSERT_TRUE(viewDef.IsValid());
    DisplayStyleR style = viewDef->GetDisplayStyle();
    ASSERT_EQ(style.GetViewFlags().GetRenderMode(), Render::RenderMode::SmoothShade);
    Render::ViewFlags flags = style.GetViewFlags();
    flags.SetRenderMode(Render::RenderMode::SolidFill);
    style.SetViewFlags(flags);    
    ASSERT_EQ(style.GetViewFlags().GetRenderMode(), Render::RenderMode::SolidFill);
    style.SetBackgroundColor(ColorDef::Red());
    ASSERT_EQ(style.GetBackgroundColor(), ColorDef::Red());
    ASSERT_EQ(viewDef->GetDescription(), "DrawingView Descr");
    viewDef->SetDescription("Descr");
    ASSERT_EQ(viewDef->GetDescription(), "Descr");
    ASSERT_TRUE(viewDef->Update().IsValid());
    ASSERT_TRUE(style.Update().IsValid());
    //SheetViewDefinition
    SheetViewDefinitionPtr SviewDef = m_db->Elements().GetForEdit<SheetViewDefinition>(SviewDefid);
    ASSERT_TRUE(SviewDef.IsValid());
    DisplayStyleR Sstyle = SviewDef->GetDisplayStyle();
    ASSERT_EQ(Sstyle.GetViewFlags().GetRenderMode(), Render::RenderMode::SolidFill);
    ASSERT_EQ(Sstyle.GetBackgroundColor(), ColorDef::Red());
    ASSERT_EQ(SviewDef->GetDescription(), "SheetView Descr");
    SviewDef->SetDescription("Descr");
    ASSERT_EQ(SviewDef->GetDescription(), "Descr");
    ASSERT_TRUE(SviewDef->Update().IsValid());
    }
    m_db->CloseDb();
    m_db = nullptr;
    //Check update values are saved in Db 
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    // DrawingViewDefinition
    DrawingViewDefinitionPtr viewDef = m_db->Elements().GetForEdit<DrawingViewDefinition>(DviewDefid);
    ASSERT_TRUE(viewDef.IsValid());
    DisplayStyleR style = viewDef->GetDisplayStyle();
    ASSERT_EQ(style.GetViewFlags().GetRenderMode(), Render::RenderMode::SolidFill);
    ASSERT_EQ(style.GetBackgroundColor(), ColorDef::Red());
    ASSERT_EQ(viewDef->GetDescription(), "Descr");
    // SheetViewDefinition
    SheetViewDefinitionPtr SviewDef = m_db->Elements().GetForEdit<SheetViewDefinition>(SviewDefid);
    ASSERT_TRUE(SviewDef.IsValid());
    ASSERT_EQ(SviewDef->GetDescription(), "Descr");
    ASSERT_TRUE(SviewDef->Update().IsValid());

    // Delete the Views
    ASSERT_EQ(DgnDbStatus::Success, viewDef->Delete());
    ASSERT_EQ(DgnDbStatus::Success, SviewDef->Delete());
    ASSERT_EQ(0, ViewDefinition::QueryCount(*m_db));
    }
#ifdef NOTNOW
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                               Ridha.Malik                   3/17
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ViewDefinitionTests, ViewDefinition3dCRUD)
    {
    SetupSeedProject();
    DgnViewId viewid;
    GridOrientationType orientation; uint32_t gridPerRef;
    DPoint2d spacing;
    Json::Value clipingJson;
    if (true) 
        {
        PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "model");
        //Insert OrthographicViewDefinition
        OrthographicViewDefinitionPtr view = InsertSpatialView(*model, "view1", false);
        ASSERT_TRUE(view->IsView3d());
        ASSERT_TRUE(view->IsSpatialView());
        ASSERT_FALSE(view->IsSheetView());
        CurveVectorPtr sec1 = CurveVector::CreateRectangle(1, 2, 3, 4, CurveVector::BOUNDARY_TYPE_Outer);
        ClipVectorPtr cliping = ClipVector::CreateFromCurveVector(*sec1, 0.1, 0.5);
        ASSERT_TRUE(cliping.IsValid());
        view->SetViewClip(cliping);
        clipingJson = cliping->ToJson();
        ASSERT_EQ(view->GetViewClip()->ToJson(), clipingJson);
        view->SetGridSettings(GridOrientationType::View, DPoint2d::From(0, 50), 5);
        view->GetGridSettings(orientation, spacing, gridPerRef);
        ASSERT_EQ(orientation, GridOrientationType::View);
        ASSERT_EQ(spacing.x, 0);
        ASSERT_EQ(spacing.y, 50);
        ASSERT_EQ(gridPerRef, 5);
        view->SetExtents(DVec3d::From(0, 1, 5));
        ASSERT_EQ(view->GetExtents(), DVec3d::From(0, 1, 5));
        view->SetEyePoint(DPoint3d::From(0, 1, 5));
        ASSERT_EQ(view->GetEyePoint(), DPoint3d::From(0, 1, 5));
        //Update the view element
        ASSERT_TRUE(view->Update().IsValid());
        viewid = view->GetViewId();
        ASSERT_TRUE(viewid == ViewDefinition::QueryViewId(m_db->GetDictionaryModel(), "view1"));
        }
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    //Check what stored in Db OrthographicViewDefinition
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    {
    //OrthographicViewDefinition
    OrthographicViewDefinitionCPtr view = m_db->Elements().Get<OrthographicViewDefinition>(viewid);
    ASSERT_TRUE(view.IsValid());
    ASSERT_EQ(view->GetViewClip()->ToJson(), clipingJson);
    view->GetGridSettings(orientation, spacing, gridPerRef);
    ASSERT_EQ(orientation, GridOrientationType::View);
    ASSERT_EQ(spacing.x, 0);
    ASSERT_EQ(spacing.y, 50);
    ASSERT_EQ(gridPerRef, 5);
    ASSERT_EQ(view->GetExtents(), DVec3d::From(0, 1, 5));
    ASSERT_EQ(view->GetEyePoint(), DPoint3d::From(0, 1, 5));
    }
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    // Delete the View
    OrthographicViewDefinitionPtr view = m_db->Elements().GetForEdit<OrthographicViewDefinition>(viewid);
    ASSERT_EQ(DgnDbStatus::Success, view->Delete());
    ASSERT_EQ(0, ViewDefinition::QueryCount(*m_db));
    }
#endif
