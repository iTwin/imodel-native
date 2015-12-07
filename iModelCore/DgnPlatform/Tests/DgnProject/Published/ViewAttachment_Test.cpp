/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ViewAttachment_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/ViewAttachment.h>

#define EXPECT_INVALID(EXPR) EXPECT_FALSE((EXPR).IsValid())

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct ViewAttachmentTest : public GenericDgnModelTestFixture
{
public:
    DEFINE_T_SUPER(GenericDgnModelTestFixture);

    ViewAttachmentTest() : T_Super(__FILE__, true, true) { }

    static Placement2d MakePlacement()
        {
        Placement2d placement(DPoint2d::From(0,0), AngleInDegrees(), ElementAlignedBox2d(0,0,1,1));
        EXPECT_TRUE(placement.IsValid());
        return placement;
        }
    static ViewAttachment::Data MakeData(DgnViewId viewId, double ox, double oy, double dx, double dy, double s)
        {
        return ViewAttachment::Data(viewId, DPoint3d::FromXYZ(ox,oy,0), DVec3d::From(dx,dy,0), s);
        }
    ViewAttachment::CreateParams MakeParams(ViewAttachment::Data const& data, DgnModelId mid, DgnCategoryId cat, Placement2dCR placement=Placement2d())
        {
        return ViewAttachment::CreateParams(*GetDgnDb(), mid, ViewAttachment::QueryClassId(*GetDgnDb()), cat, data, placement);
        }
    ViewAttachmentCPtr InsertAttachment(ViewAttachment::CreateParams const& params)
        {
        ViewAttachment attachment(params);
        return attachment.Insert();
        }
    ViewAttachmentCPtr UpdateAttachment(ViewAttachmentCR in, ViewAttachment::Data const& data)
        {
        auto pAttach = in.MakeCopy<ViewAttachment>();
        pAttach->SetViewParams(data.m_origin, data.m_delta, data.m_scale);
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
        ExpectEqualPoints(attach.GetViewOrigin(), data.m_origin);
        ExpectEqualPoints(attach.GetViewDelta(), data.m_delta);
        EXPECT_EQ(attach.GetViewId(), data.m_viewId);
        EXPECT_EQ(attach.GetViewScale(), data.m_scale);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ViewAttachmentTest, CRUD)
    {
    auto& db = *GetDgnDb();
    DgnModelId drawingId, sheetId;
    DgnCategoryId attachmentCatId;
    DgnViewId viewId;

        {
        // Set up a sheet to hold attachments
        DgnClassId classId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SheetModel));
        SheetModelPtr sheet = new SheetModel(SheetModel::CreateParams(db, classId, DgnModel::CreateModelCode("MySheet"), DPoint2d::From(10,10)));
        ASSERT_EQ(DgnDbStatus::Success, sheet->Insert());
        sheetId = sheet->GetModelId();

        // Set up a category for attachments
        DgnCategory cat(DgnCategory::CreateParams(db, "Attachments", DgnCategory::Scope::Annotation));
        cat.Insert(DgnSubCategory::Appearance());
        attachmentCatId = cat.GetCategoryId();
        ASSERT_TRUE(attachmentCatId.IsValid());

        // Set up a viewed model
        classId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SectionDrawingModel));
        RefCountedPtr<SectionDrawingModel> drawing = new SectionDrawingModel(SectionDrawingModel::CreateParams(db, classId, DgnModel::CreateModelCode("MyDrawing")));
        ASSERT_EQ(DgnDbStatus::Success, drawing->Insert());
        drawingId = drawing->GetModelId();

        // Create a view of our (empty) model
        DrawingViewDefinition view(DrawingViewDefinition::CreateParams(db, "MyDrawingView", DrawingViewDefinition::Data(drawingId)));
        view.Insert();
        viewId = view.GetViewId();
        ASSERT_TRUE(viewId.IsValid());
        }

    // Test some invalid CreateParams

    // Invalid view id
    EXPECT_INVALID(InsertAttachment(MakeParams(MakeData(DgnViewId(),0,0,1,1,1), sheetId, attachmentCatId, MakePlacement())));
    // Invalid category
    EXPECT_INVALID(InsertAttachment(MakeParams(MakeData(viewId,0,0,1,1,1), sheetId, DgnCategoryId(), MakePlacement())));
    // Not a sheet model
    EXPECT_INVALID(InsertAttachment(MakeParams(MakeData(viewId,0,0,1,1,1), drawingId, attachmentCatId, MakePlacement())));
    // Negative scale
    EXPECT_INVALID(InsertAttachment(MakeParams(MakeData(viewId,0,0,1,1,-1), sheetId, attachmentCatId, MakePlacement())));
    // Zero delta
    EXPECT_INVALID(InsertAttachment(MakeParams(MakeData(viewId,0,0,0,0,1), sheetId, attachmentCatId, MakePlacement())));

    // Create a valid view attachment
    auto data = MakeData(viewId, -5.0, 2.5, 1.0, 0.5, 3.0);
    auto cpAttach = InsertAttachment(MakeParams(data, sheetId, attachmentCatId, MakePlacement()));
    EXPECT_TRUE(cpAttach.IsValid());

    // Confirm data as expected
    ExpectData(*cpAttach, data);

    // Modify
    data.m_origin.x += 0.25;
    data.m_delta.y -= 0.25;
    data.m_scale *= 2;
    cpAttach = UpdateAttachment(*cpAttach, data);
    ExpectData(*cpAttach, data);

    // Deleting the view definition deletes attachments which reference it
    DgnElementId attachId = cpAttach->GetElementId();
    EXPECT_TRUE(db.Elements().GetElement(attachId).IsValid());

    auto view = ViewDefinition::QueryView(viewId, db);
    EXPECT_EQ(DgnDbStatus::Success, view->Delete());
    EXPECT_EQ(BE_SQLITE_OK, db.SaveChanges());

    EXPECT_INVALID(db.Elements().GetElement(attachId));
    EXPECT_INVALID(db.Elements().GetElement(viewId));
    }

