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
    static ViewAttachment::Data MakeData(DgnViewId viewId, double ox, double oy, double dx, double dy, double s)
        {
        return ViewAttachment::Data(viewId, s);
        }
    ViewAttachment::CreateParams MakeParams(ViewAttachment::Data const& data, DgnModelId mid, DgnCategoryId cat, Placement2dCR placement=Placement2d())
        {
        return ViewAttachment::CreateParams(GetDgnDb(), mid, ViewAttachment::QueryClassId(GetDgnDb()), cat, data, placement);
        }
    ViewAttachmentCPtr InsertAttachment(ViewAttachment::CreateParams const& params)
        {
        ViewAttachment attachment(params);
        return attachment.Insert();
        }
    ViewAttachmentCPtr UpdateAttachment(ViewAttachmentCR in, ViewAttachment::Data const& data)
        {
        auto pAttach = in.MakeCopy<ViewAttachment>();
        pAttach->SetViewScale(data.m_scale);
        return pAttach->Update();
        }

    void ExpectEqualPoints(DPoint3dCR a, DPoint3dCR b)
        {
        EXPECT_EQ(a.x, b.x);
        EXPECT_EQ(a.y, b.y);
        EXPECT_EQ(a.z, b.z);
        }

    void ExpectData(ViewAttachmentCR attach, ViewAttachment::Data const& data)
        {
        EXPECT_EQ(attach.GetViewId(), data.m_viewId);
        EXPECT_EQ(attach.GetViewScale(), data.m_scale);
        }

    DrawingViewDefinition& GetDrawingViewDef(DgnDbR db) {return const_cast<DrawingViewDefinition&>(*ViewDefinition::QueryView(m_viewId, db)->ToDrawingView());}

    void AddTextToDrawing(DgnModelId drawingId, Utf8CP text="My Text", double viewRot=0.0);
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
    DocumentListModelPtr sheetListModel = DgnDbTestUtils::InsertDocumentListModel(db, DgnModel::CreateModelCode("SheetListModel"));
    SheetPtr sheet = DgnDbTestUtils::InsertSheet(*sheetListModel, 1.0,1.0,1.0, DgnCode(), "MySheet");
    SheetModelPtr sheetModel = DgnDbTestUtils::InsertSheetModel(*sheet, DgnModel::CreateModelCode("MySheetModel"));
    m_sheetModelId = sheetModel->GetModelId();

    // Set up a category for attachments
    DgnCategory cat(DgnCategory::CreateParams(db, "Attachments", DgnCategory::Scope::Annotation));
    cat.Insert(DgnSubCategory::Appearance());
    m_attachmentCatId = cat.GetCategoryId();
    ASSERT_TRUE(m_attachmentCatId.IsValid());

    // Set up a viewed model
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(db, DgnModel::CreateModelCode("MyDrawingListModel"));
    SectionDrawingPtr drawing = DgnDbTestUtils::InsertSectionDrawing(*drawingListModel, DgnCode(), "MySectionDrawing");
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing, DgnModel::CreateModelCode("MyDrawingModel"));
    m_drawingModelId = drawing->GetModelId();

    // Create a view of our (empty) model
    DrawingViewDefinition view(db, "MyDrawingView", m_drawingModelId, CategorySelector(db,""), DisplayStyle(db,""));
    view.Insert();
    m_viewId = view.GetViewId();
    ASSERT_TRUE(m_viewId.IsValid());
    db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentTest::AddTextToDrawing(DgnModelId drawingId, Utf8CP text, double viewRot)
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

    TextAnnotation anno(db);
    anno.SetText(AnnotationTextBlock::Create(db, m_textStyleId, text).get());
    TextAnnotation2dPtr annoElem = new TextAnnotation2d(TextAnnotation2d::CreateParams(db, drawingId, TextAnnotation2d::QueryDgnClassId(db), m_attachmentCatId));
    annoElem->SetAnnotation(&anno);
    EXPECT_TRUE(annoElem->Insert().IsValid());

    DrawingViewControllerPtr viewController = GetDrawingViewDef(db).LoadViewController();
    SetupAndSaveViewController(*viewController, *annoElem, drawingId, viewRot);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachmentTest::AddBoxToDrawing(DgnModelId drawingId, double width, double height, double viewRot)
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
    DgnElementPtr el = dgn_ElementHandler::Element::FindHandler(db, classId)->Create(DgnElement::CreateParams(db, drawingId, classId, DgnCode()));
    ASSERT_TRUE(el.IsValid());

    auto geomEl = el->ToGeometrySourceP()->ToGeometrySource2dP();
    geomEl->SetCategoryId(m_attachmentCatId);
    geomEl->SetPlacement(Placement2d(DPoint2d::From(3,2), AngleInDegrees()));
    GeometryBuilderPtr builder = GeometryBuilder::Create(*geomEl);

    builder->Append(*curve);
    EXPECT_EQ(SUCCESS, builder->Finish(*geomEl));
    EXPECT_TRUE(el->Insert().IsValid());

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
    viewController.GetViewFlagsR().SetRenderMode(Render::RenderMode::Wireframe);
    viewController.ChangeCategoryDisplay(m_attachmentCatId, true);

//    viewController.ChangeModelDisplay(modelId, true);

    ASSERT_TRUE(viewController.GetViewDefinitionR().Update().IsValid());
    ASSERT_TRUE(viewController.GetViewDefinitionR().GetCategorySelectorR().Update().IsValid());
    ASSERT_TRUE(viewController.GetViewDefinitionR().GetDisplayStyleR().Update().IsValid());
    }

#if defined (NEEDS_WORK_TARGET_MODEL)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ViewAttachmentTest, CRUD)
    {
    auto& db = GetDgnDb();

    // Test some invalid CreateParams

    // Invalid view id
    EXPECT_INVALID(InsertAttachment(MakeParams(MakeData(DgnViewId(),0,0,1,1,1), m_sheetModelId, m_attachmentCatId, MakePlacement())));
    // Invalid category
    EXPECT_INVALID(InsertAttachment(MakeParams(MakeData(m_viewId,0,0,1,1,1), m_sheetModelId, DgnCategoryId(), MakePlacement())));
    // Not a sheet model
    EXPECT_INVALID(InsertAttachment(MakeParams(MakeData(m_viewId,0,0,1,1,1), m_drawingModelId, m_attachmentCatId, MakePlacement())));
    // Negative scale
    EXPECT_INVALID(InsertAttachment(MakeParams(MakeData(m_viewId,0,0,1,1,-1), m_sheetModelId, m_attachmentCatId, MakePlacement())));

    // Create a valid view attachment
    auto data = MakeData(m_viewId, -5.0, 2.5, 1.0, 0.5, 3.0);
    auto cpAttach = InsertAttachment(MakeParams(data, m_sheetModelId, m_attachmentCatId, MakePlacement()));
    EXPECT_TRUE(cpAttach.IsValid());

    // Confirm data as expected
    ExpectData(*cpAttach, data);

    // Modify
    data.m_scale *= 2;
    cpAttach = UpdateAttachment(*cpAttach, data);
    ExpectData(*cpAttach, data);

    // Deleting the view definition deletes attachments which reference it
    DgnElementId attachId = cpAttach->GetElementId();
    EXPECT_TRUE(db.Elements().GetElement(attachId).IsValid());

    auto view = ViewDefinition::QueryView(m_viewId, db);
    EXPECT_EQ(DgnDbStatus::Success, view->Delete());
    EXPECT_EQ(BE_SQLITE_OK, db.SaveChanges());

    EXPECT_INVALID(db.Elements().GetElement(attachId));
    EXPECT_INVALID(db.Elements().GetElement(m_viewId));
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
    static const double scale = 2.0;
    ViewAttachment::Data data(m_viewId, scale);
    auto cpAttach = InsertAttachment(MakeParams(data, m_sheetModelId, m_attachmentCatId, MakePlacement()));
    ASSERT_TRUE(cpAttach.IsValid());

    ViewAttachmentPtr pAttach = cpAttach->MakeCopy<ViewAttachment>();
    ASSERT_TRUE(pAttach.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, pAttach->GenerateGeomStream());
    cpAttach = pAttach->Update();
    EXPECT_TRUE(cpAttach.IsValid());

    SheetViewDefinition sheetView(db, "MySheetView", m_sheetModelId, CategorySelector(db,""), DisplayStyle(db,""));
    sheetView.Insert();

    SheetViewControllerPtr viewController = sheetView.LoadViewController();
    SetupAndSaveViewController(*viewController, *cpAttach, m_sheetModelId);

    db.SaveChanges();
    }
#endif

