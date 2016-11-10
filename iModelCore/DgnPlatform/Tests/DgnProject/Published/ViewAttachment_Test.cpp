/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ViewAttachment_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/ViewAttachment.h>
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatform/Annotations/TextAnnotationElement.h>

#define EXPECT_INVALID(EXPR) EXPECT_FALSE((EXPR).IsValid())

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ViewAttachmentTest : public DgnDbTestFixture
{
protected:
    DgnModelId m_drawingModelId;
    DgnModelId m_sheetModelId;
    DgnCategoryId m_attachmentCatId;
    DgnCategoryId m_annotationCatId;
    DgnViewId m_viewId;
    DgnElementId m_textStyleId;
public:
    DEFINE_T_SUPER(GenericDgnModelTestFixture);

    ViewAttachmentTest()  { }

    virtual void SetUp() override;

    static Placement2d MakePlacement()
        {
        Placement2d placement(DPoint2d::From(0,0), AngleInDegrees(), ElementAlignedBox2d(0,0,1,1));
        EXPECT_TRUE(placement.IsValid());
        return placement;
        }
    void ExpectEqualPoints(DPoint3dCR a, DPoint3dCR b)
        {
        EXPECT_EQ(a.x, b.x);
        EXPECT_EQ(a.y, b.y);
        EXPECT_EQ(a.z, b.z);
        }

    DrawingViewDefinition& GetDrawingViewDef(DgnDbR db) {return const_cast<DrawingViewDefinition&>(*ViewDefinition::QueryView(m_viewId, db)->ToDrawingView());}

    void AddTextToModel(TextAnnotation2dCPtr&, DgnModelId, DPoint2dCR origin, Utf8CP text, double textRotationDegrees=0.0);
    void AddTextToDrawing(DgnModelId drawingId, Utf8CP text="My Text", double viewRot=0.0);
    void AddBoxToModel(AnnotationElement2dCPtr&, DgnModelId, DPoint2dCR origin, double width, double height, double boxRotationDegrees=0.0);
    void AddBoxToDrawing(DgnModelId drawingId, double width, double height, double viewRot=0.0);
    template<typename VC, typename EL> void SetupAndSaveViewController(VC& viewController, EL const& el, DgnModelId modelId, double rot=0.0);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentTest::SetUp()
    {
    SetupSeedProject();
    DgnDbR db = GetDgnDb();

    // Set up a sheet to hold attachments
    DocumentListModelPtr sheetListModel = DgnDbTestUtils::InsertDocumentListModel(db, "SheetListModel");
    SheetPtr sheet = DgnDbTestUtils::InsertSheet(*sheetListModel, 1.0,1.0,1.0, "MySheet");
    SheetModelPtr sheetModel = DgnDbTestUtils::InsertSheetModel(*sheet);
    m_sheetModelId = sheetModel->GetModelId();

    // Set up a category for attachments
    m_attachmentCatId = DgnDbTestUtils::InsertCategory(db, "Attachments", ColorDef::Cyan(), DgnCategory::Scope::Annotation);
    ASSERT_TRUE(m_attachmentCatId.IsValid());
    m_annotationCatId = DgnDbTestUtils::InsertCategory(db, "Annotations", ColorDef::Cyan(), DgnCategory::Scope::Annotation);
    ASSERT_TRUE(m_annotationCatId.IsValid());

    // Set up a viewed model
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(db, "MyDrawingListModel");
    SectionDrawingPtr drawing = DgnDbTestUtils::InsertSectionDrawing(*drawingListModel, "MySectionDrawing");
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    m_drawingModelId = drawingModel->GetModelId();

    // Create a view of our (empty) model
    DrawingViewDefinition view(db, "MyDrawingView", m_drawingModelId, *new CategorySelector(db,""), *new DisplayStyle(db,""));
    view.Insert();
    m_viewId = view.GetViewId();
    ASSERT_TRUE(m_viewId.IsValid());
    db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentTest::AddTextToModel(TextAnnotation2dCPtr& annoElemOut, DgnModelId modelId, DPoint2dCR origin, Utf8CP text, double textRotationDegrees)
    {
    auto& db = GetDgnDb();
    if (!m_textStyleId.IsValid())
        {
        AnnotationTextStyle style(db);
        style.SetName("MyTextStyle");
        style.SetFontId(db.Fonts().AcquireId(DgnFontManager::GetLastResortTrueTypeFont()));
        style.SetHeight(1.0);
        style.Insert();
        m_textStyleId = style.GetElementId();
        EXPECT_TRUE(m_textStyleId.IsValid());
        }

    ASSERT_TRUE(m_annotationCatId.IsValid());

    TextAnnotation anno(db);
    anno.SetText(AnnotationTextBlock::Create(db, m_textStyleId, text).get());
    TextAnnotation2dPtr annoElem = new TextAnnotation2d(TextAnnotation2d::CreateParams(db, modelId, TextAnnotation2d::QueryDgnClassId(db), m_annotationCatId));
    annoElem->SetAnnotation(&anno);

    auto placement = annoElem->GetPlacement();
    placement.GetOriginR() = origin;
    placement.GetAngleR() = AngleInDegrees::FromDegrees(textRotationDegrees);

    annoElemOut = db.Elements().Insert(*annoElem);
    ASSERT_TRUE(annoElemOut.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentTest::AddTextToDrawing(DgnModelId drawingId, Utf8CP text, double viewRot)
    {
    auto& db = GetDgnDb();
    TextAnnotation2dCPtr annoElem;
    AddTextToModel(annoElem, drawingId, DPoint2d::From(3,2), text);
    DrawingViewControllerPtr viewController = GetDrawingViewDef(db).LoadViewController();
    SetupAndSaveViewController(*viewController, *annoElem, drawingId, viewRot);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentTest::AddBoxToModel(AnnotationElement2dCPtr& geomElOut, DgnModelId modelId, DPoint2dCR origin, double width, double height, double rot)
    {
    bvector<DPoint3d> pts
        {
        DPoint3d::FromXYZ(0,0,0),
        DPoint3d::FromXYZ(0,height,0),
        DPoint3d::FromXYZ(width,height,0),
        DPoint3d::FromXYZ(width,0,0),
        };

    ICurvePrimitivePtr curve = ICurvePrimitive::CreateLineString(pts);

    auto& db = GetDgnDb();
    DgnClassId classId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationElement2d));
    DgnElementPtr el = dgn_ElementHandler::Element::FindHandler(db, classId)->Create(DgnElement::CreateParams(db, modelId, classId, DgnCode()));
    ASSERT_TRUE(el.IsValid());

    auto geomEl = el->ToGeometrySourceP()->GetAsGeometrySource2dP();
    geomEl->SetCategoryId(m_annotationCatId);
    geomEl->SetPlacement(Placement2d(origin, AngleInDegrees::FromDegrees(rot)));
    GeometryBuilderPtr builder = GeometryBuilder::Create(*geomEl);

    builder->Append(*curve);
    EXPECT_EQ(SUCCESS, builder->Finish(*geomEl));

    auto persistentEl = db.Elements().Insert(*el);
    ASSERT_TRUE(persistentEl.IsValid());

    geomElOut = persistentEl->ToAnnotationElement2d();
    ASSERT_TRUE(geomElOut.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentTest::AddBoxToDrawing(DgnModelId drawingId, double width, double height, double viewRot)
    {
    auto& db = GetDgnDb();
    AnnotationElement2dCPtr geomEl;
    AddBoxToModel(geomEl, drawingId, DPoint2d::From(3,2), width, height, viewRot);
    DrawingViewControllerPtr viewController = GetDrawingViewDef(db).LoadViewController();
    SetupAndSaveViewController(*viewController, *geomEl, drawingId, viewRot);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename VC, typename EL> void ViewAttachmentTest::SetupAndSaveViewController(VC& viewController, EL const& el, DgnModelId modelId, double rot)
    {
    // Set up the view to display the new element...
    ViewDefinition::MarginPercent viewMargin(.1,.1,.1,.1);
    viewController.SetStandardViewRotation(StandardView::Top);
    viewController.SetRotation(RotMatrix::FromAxisAndRotationAngle(2, rot));
    viewController.LookAtVolume(el.CalculateRange3d(), nullptr, &viewMargin);

    auto flags = viewController.GetViewFlags();
    flags.SetRenderMode(Render::RenderMode::Wireframe);
    viewController.GetViewDefinition().GetDisplayStyle().SetViewFlags(flags);

    viewController.ChangeCategoryDisplay(el.GetCategoryId(), true);

//    viewController.ChangeModelDisplay(modelId, true);

    ASSERT_TRUE(viewController.GetViewDefinition().Update().IsValid());
    ASSERT_TRUE(viewController.GetViewDefinition().GetCategorySelector().Update().IsValid());
    ASSERT_TRUE(viewController.GetViewDefinition().GetDisplayStyle().Update().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ViewAttachmentTest, CRUD)
    {
    auto& db = GetDgnDb();

    Placement2d placement = MakePlacement();

    // Test some invalid CreateParams
    // Invalid view id
    {
    ViewAttachment attachment(GetDgnDb(), m_sheetModelId, DgnViewId(), m_attachmentCatId, placement);
    EXPECT_INVALID(attachment.Insert());
    }
    // Invalid category
    {
    ViewAttachment attachment(GetDgnDb(), m_sheetModelId, m_viewId, DgnCategoryId(), placement);
    EXPECT_INVALID(attachment.Insert());
    }
    // Not a sheet model
    {
    ViewAttachment attachment(GetDgnDb(), m_drawingModelId, m_viewId, m_attachmentCatId, placement);
    EXPECT_INVALID(attachment.Insert());
    }

    // Create a valid attachment attachment
    ViewAttachment attachment(GetDgnDb(), m_sheetModelId, m_viewId, m_attachmentCatId, placement);
    auto cpAttach = GetDgnDb().Elements().Insert(attachment);
    ASSERT_TRUE(cpAttach.IsValid());

    // Confirm data as expected
    EXPECT_EQ(m_viewId, cpAttach->GetViewId());
    EXPECT_TRUE(placement.GetOrigin().IsEqual(cpAttach->GetPlacement().GetOrigin()));

    // Modify
    placement.GetOriginR().Add(DVec2d::From(0,1));
    attachment.SetPlacement(placement);
    cpAttach = GetDgnDb().Elements().Update(attachment);
    EXPECT_TRUE(placement.GetOrigin().IsEqual(cpAttach->GetPlacement().GetOrigin()));

    // Deleting the attachment definition deletes attachments which reference it
    DgnElementId attachId = cpAttach->GetElementId();
    EXPECT_TRUE(db.Elements().GetElement(attachId).IsValid());

#ifdef WIP_ATTACHMENTS // *** NavigationProperty currently prevents me from doing this
    auto view = ViewDefinition::QueryView(m_viewId, db);
    EXPECT_EQ(DgnDbStatus::Success, view->Delete());
    EXPECT_EQ(BE_SQLITE_OK, db.SaveChanges());

    EXPECT_INVALID(db.Elements().GetElement(attachId));
    EXPECT_INVALID(db.Elements().GetElement(m_viewId));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ViewAttachmentTest, Geom)
    {
    auto& db = GetDgnDb();

    // Add some geometry to the drawing and regenerate attachment geometry
    static const double drawingViewRot = /*45.0*msGeomConst_piOver2*/ 0.0;

    AddTextToDrawing(m_drawingModelId, "Text", drawingViewRot);
    AddBoxToDrawing(m_drawingModelId, 5, 10, drawingViewRot);

    // Create an attachment
    Placement2d placement(DPoint2d::From(0,0), AngleInDegrees(), ElementAlignedBox2d(0,0,1,1));
    ViewAttachment attachment(GetDgnDb(), m_sheetModelId, m_viewId, m_attachmentCatId, placement);
    auto cpAttach = GetDgnDb().Elements().Insert(attachment);
    ASSERT_TRUE(cpAttach.IsValid());

    TextAnnotation2dCPtr annoElemOnSheet;
    AddTextToModel(annoElemOnSheet, m_sheetModelId, DPoint2d::From(0,0), "Text on sheet");

    AnnotationElement2dCPtr boxOnSheet; // make a box about the same size as the ViewAttachment, so that we can use it to check the attachment's BB by eye
    AddBoxToModel(boxOnSheet, m_sheetModelId, DPoint2d::From(0,0), placement.GetElementBox().GetWidth()+0.01, placement.GetElementBox().GetHeight()+0.01);

    ViewAttachmentPtr pAttach = cpAttach->MakeCopy<ViewAttachment>();
    ASSERT_TRUE(pAttach.IsValid());

    DisplayStylePtr noStyle = new DisplayStyle(db,"");
    CategorySelectorPtr cats = new CategorySelector(db,"");
    SheetViewDefinition sheetView(db, "MySheetView", m_sheetModelId, *cats, *noStyle);
    sheetView.Insert();

    SheetViewControllerPtr viewController = sheetView.LoadViewController();
    SetupAndSaveViewController(*viewController, *cpAttach, m_sheetModelId);

    viewController->ChangeCategoryDisplay(m_attachmentCatId, true);
    viewController->ChangeCategoryDisplay(m_annotationCatId, true);
    viewController->GetViewDefinition().GetCategorySelector().Update();

    db.SaveChanges();
    }
