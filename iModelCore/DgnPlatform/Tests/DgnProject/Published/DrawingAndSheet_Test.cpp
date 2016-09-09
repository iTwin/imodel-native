/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DrawingAndSheet_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Shaun.Sewall                    09/2016
//----------------------------------------------------------------------------------------
struct DrawingAndSheetTests : public DgnDbTestFixture
    {
    };

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
TEST_F(DrawingAndSheetTests, BasicCRUD)
    {
    SetupSeedProject();

    SubjectCPtr rootSubject = m_db->Elements().GetRootSubject();
    ASSERT_TRUE(rootSubject.IsValid());

    SubjectCPtr drawingListSubject = Subject::CreateAndInsert(*rootSubject, "DrawingListSubject");
    ASSERT_TRUE(drawingListSubject.IsValid());

    SubjectCPtr sheetListSubject = Subject::CreateAndInsert(*rootSubject, "SheetListSubject");
    ASSERT_TRUE(sheetListSubject.IsValid());

    DocumentListModelPtr drawingListModel = DocumentListModel::CreateAndInsert(*drawingListSubject, DgnModel::CreateModelCode("DrawingListModel"));
    ASSERT_TRUE(drawingListModel.IsValid());
    
    DocumentListModelPtr sheetListModel = DocumentListModel::CreateAndInsert(*sheetListSubject, DgnModel::CreateModelCode("SheetListModel"));
    ASSERT_TRUE(sheetListModel.IsValid());

    for (int i=0; i<4; i++)
        {
        DrawingPtr drawing = Drawing::Create(*drawingListModel, DgnCode(), Utf8PrintfString("Drawing%d", i).c_str());
        ASSERT_TRUE(drawing.IsValid());
        ASSERT_TRUE(drawing->Insert().IsValid());

        DrawingModelPtr drawingModel = DrawingModel::Create(*drawing, DgnModel::CreateModelCode(Utf8PrintfString("DrawingModel%d", i).c_str()));
        ASSERT_TRUE(drawingModel.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, drawingModel->Insert());
        }

    for (int i=0; i<3; i++)
        {
        SheetPtr sheet = Sheet::Create(*sheetListModel, DgnCode(), Utf8PrintfString("Sheet%d", i).c_str());
        ASSERT_TRUE(sheet.IsValid());
        ASSERT_TRUE(sheet->Insert().IsValid());

        SheetModelPtr sheetModel = SheetModel::Create(*sheet, DgnModel::CreateModelCode(Utf8PrintfString("SheetModel%d", i).c_str()));
        ASSERT_TRUE(sheetModel.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, sheetModel->Insert());
        }
    }
