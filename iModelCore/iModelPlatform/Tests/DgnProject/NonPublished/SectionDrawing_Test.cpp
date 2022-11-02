/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnDbTables.h>

USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
// @bsistruct
//=======================================================================================
struct SectionDrawingTest : public DgnDbTestFixture
{
protected:
    DgnDbR CloseAndReopenDb()
        {
        auto filename = BeFileName(m_db->GetDbFileName());
        m_db->CloseDb();
        OpenDb(m_db, filename, BeSQLite::Db::OpenMode::ReadWrite);
        return *m_db;
        }
public:
    SectionDrawingTest() { }

    void SetUp() override { SetupSeedProject(); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SectionDrawingTest, CreateSectionDrawing)
    {
    auto sectionType = SectionType::Elevation;
    DgnViewId spatialViewId(uint64_t(0x123));
    auto drawingToSpatial = Transform::FromScaleFactors(1.0, 5.0, 20.0);
    auto sheetToSpatial = Transform::From(DPoint3d::From(1.0, 5.0, 20.0));
    DgnElementId drawingId;
        {
        // Insert SectionDrawing
        auto& db = GetDgnDb();
        auto drawingListModel = DgnDbTestUtils::InsertDocumentListModel(db, "DrawingList");
        auto drawing = DgnDbTestUtils::InsertSectionDrawing(*drawingListModel, "SectionDrawing");

        // Expect default values
        EXPECT_EQ(drawing->GetSectionType(), SectionType::Section);
        EXPECT_FALSE(drawing->GetSpatialViewId().IsValid());

        Transform transform;
        EXPECT_FALSE(drawing->GetDrawingToSpatialTransform(transform));
        EXPECT_FALSE(drawing->GetSheetToSpatialTransform(transform));

        // Modify values
        EXPECT_EQ(DgnDbStatus::Success, drawing->SetSectionType(sectionType));
        EXPECT_EQ(DgnDbStatus::Success, drawing->SetSpatialViewId(spatialViewId));
        drawing->SetDrawingToSpatialTransform(drawingToSpatial);
        drawing->SetSheetToSpatialTransform(sheetToSpatial);

        // Expected updated values
        EXPECT_EQ(drawing->GetSectionType(), sectionType);
        EXPECT_EQ(drawing->GetSpatialViewId(), spatialViewId);
        EXPECT_TRUE(drawing->GetDrawingToSpatialTransform(transform));
        EXPECT_TRUE(transform.IsEqual(drawingToSpatial));
        EXPECT_TRUE(drawing->GetSheetToSpatialTransform(transform));
        EXPECT_TRUE(transform.IsEqual(sheetToSpatial));

        // Persist changes
        DgnDbStatus updateStatus = drawing->Update();
        EXPECT_EQ(DgnDbStatus::Success, updateStatus);

        drawingId = drawing->GetElementId();
        db.SaveChanges();
        }

    // Obtain persistent SectionDrawing
    auto& db = CloseAndReopenDb();
    auto drawing = db.Elements().Get<SectionDrawing>(drawingId);
    EXPECT_TRUE(drawing.IsValid());

    // Expect persistent values
    Transform transform;
    EXPECT_EQ(drawing->GetSectionType(), sectionType);
    EXPECT_EQ(drawing->GetSpatialViewId(), spatialViewId);
    EXPECT_TRUE(drawing->GetDrawingToSpatialTransform(transform));
    EXPECT_TRUE(transform.IsEqual(drawingToSpatial));
    EXPECT_TRUE(drawing->GetSheetToSpatialTransform(transform));
    EXPECT_TRUE(transform.IsEqual(sheetToSpatial));
    }
