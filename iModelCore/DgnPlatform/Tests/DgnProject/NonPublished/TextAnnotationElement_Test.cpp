/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "AnnotationTestFixture.h"
#include <DgnPlatform/Annotations/TextAnnotationElement.h>

USING_NAMESPACE_BENTLEY_SQLITE
/*---------------------------------------------------------------------------------**//**
* @bsistruct                                            Umar.Hayat      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct TextAnnotationTest : DgnDbTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
static DgnElementId ensureAnnotationTextStyle1(DgnDbR db)
    {
    static const Utf8CP STYLE_NAME = "AnnotationTextStyle1";
    AnnotationTextStyleCPtr existingStyle = AnnotationTextStyle::Get(db.GetDictionaryModel(), STYLE_NAME);
    if (existingStyle.IsValid())
        return existingStyle->GetElementId();

    AnnotationTextStyle style(db.GetDictionaryModel());
    style.SetColorType(AnnotationColorType::RGBA);
    style.SetColorValue(ColorDef(0x00, 0xff, 0x00));
    style.SetFontId(db.Fonts().AcquireId(DgnFontManager::GetAnyLastResortFont()));
    style.SetHeight(1000.0);
    style.SetName(STYLE_NAME);

    style.Insert();

    return style.GetElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
TEST_F(TextAnnotationTest, BasicCrud2d)
    {
    // The goal of this is to exercise persistence into and out of the database.
    // To defeat element caching, liberally open and close the DB.

    BeFileName dbPath;
    if (true)
        {
        SetupSeedProject();
        dbPath = BeFileName(m_db->GetDbFileName());
        }

    DgnModelId modelId;
    DgnElementId textStyleId;
    DgnElementId insertedElementId;
    static Utf8CP ANNOTATION_TEXT_1 = "Hello world.";
    static Utf8CP ANNOTATION_TEXT_2 = "Lorem ipsum dolar sit amet.";

    // Write the element to the database.
    //.............................................................................................
    {
    DbResult openStatus;
    DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(BE_SQLITE_OK == openStatus);
    ASSERT_TRUE(db.IsValid());

    DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(*db, "Annotation Category");
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*db, "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "2D Drawing");
    DrawingModelPtr model = DgnDbTestUtils::InsertDrawingModel(*drawing);
    modelId = model->GetModelId();

    textStyleId = ensureAnnotationTextStyle1(*db);
    ASSERT_TRUE(textStyleId.IsValid());

    TextAnnotation annotation(*db);
    annotation.SetText(AnnotationTextBlock::Create(*db, textStyleId, ANNOTATION_TEXT_1).get());
    ASSERT_TRUE(nullptr != annotation.GetTextCP());

    //.........................................................................................
    TextAnnotation2dPtr annotationElement = new TextAnnotation2d(TextAnnotation2d::CreateParams(*db, modelId, TextAnnotation2d::QueryDgnClassId(*db), categoryId));
    annotationElement->SetAnnotation(&annotation);
    ASSERT_TRUE(nullptr != annotationElement->GetAnnotation());

    TextAnnotation2dCPtr insertedAnnotationElement = annotationElement->Insert();
    ASSERT_TRUE(insertedAnnotationElement.IsValid());

    insertedElementId = insertedAnnotationElement->GetElementId();
    ASSERT_TRUE(insertedElementId.IsValid());

    // This is only here to aid in debugging so you can open the file in a viewer and see the element you just created.
    //.........................................................................................
        // auto viewDef = DrawingViewDefinition::MakeViewOfModel(*model, "TextAnnotation2dTest-BasicCrud");
        // EXPECT_TRUE(viewDef->Insert().IsValid());

    db->SaveChanges();
    }

    // Read the element back out, modify, and rewrite.
    //.............................................................................................
    {
    DbResult openStatus;
    DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(BE_SQLITE_OK == openStatus);
    ASSERT_TRUE(db.IsValid());

    //.........................................................................................
    TextAnnotation2dCPtr annotationElementC = TextAnnotation2d::Get(*db, insertedElementId);
    ASSERT_TRUE(annotationElementC.IsValid());

    // Spot check some properties; rely on other TextAnnotation tests to more fully test serialization, which should be relatively pass/fail on the element itself.
    TextAnnotationCP existingAnnotation = annotationElementC->GetAnnotation();
    ASSERT_TRUE(nullptr != existingAnnotation);
    EXPECT_TRUE(nullptr == existingAnnotation->GetFrameCP());
    EXPECT_TRUE(0 == existingAnnotation->GetLeaders().size());

    AnnotationTextBlockCP existingText = existingAnnotation->GetTextCP();
    ASSERT_TRUE(nullptr != existingText);
    EXPECT_TRUE(textStyleId == existingText->GetStyleId());

    AnnotationParagraphCollectionCR existingParagraphs = existingText->GetParagraphs();
    ASSERT_TRUE(1 == existingParagraphs.size());

    AnnotationRunCollectionCR existingRuns = existingParagraphs[0]->GetRuns();
    ASSERT_TRUE(1 == existingRuns.size());

    ASSERT_TRUE(AnnotationRunType::Text == existingRuns[0]->GetType());
    EXPECT_TRUE(0 == strcmp(ANNOTATION_TEXT_1, ((AnnotationTextRunCP) existingRuns[0].get())->GetContent().c_str()));

    // Update Annotation Element ( update text, and frame and leader )
    //.........................................................................................
    AnnotationFrameStylePtr frameStyle = AnnotationTestFixture::createAnnotationFrameStyle(*db, "TestFrameStyle");
    AnnotationFramePtr frame = AnnotationFrame::Create(*db, frameStyle->GetElementId());
    ASSERT_TRUE(frame.IsValid());

    AnnotationLeaderStylePtr leaderStyle = AnnotationTestFixture::createAnnotationLeaderStyle(*db, "TestLeaderStyle");
    AnnotationLeaderPtr leader = AnnotationLeader::Create(*db, leaderStyle->GetElementId());
    leader->SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType::PhysicalPoint);
    leader->SetSourceAttachmentType(AnnotationLeaderSourceAttachmentType::Id);
    ASSERT_TRUE(leader.IsValid());

    TextAnnotation annotation(*db);
    annotation.SetText(AnnotationTextBlock::Create(*db, textStyleId, ANNOTATION_TEXT_2).get());
    annotation.SetFrame(frame.get());
    ASSERT_TRUE(nullptr != annotation.GetFrameP());
    annotation.m_leaders.push_back(leader);

    TextAnnotation2dPtr annotationElement = TextAnnotation2d::GetForEdit(*db, insertedElementId);
    ASSERT_TRUE(annotationElement.IsValid());

    annotationElement->SetAnnotation(&annotation);

    TextAnnotation2dCPtr updatedAnnotationElement = annotationElement->Update();
    ASSERT_TRUE(updatedAnnotationElement.IsValid());
    EXPECT_TRUE(updatedAnnotationElement->GetElementId().IsValid());

    db->SaveChanges();
    }

    // Verify the modified element.
    //.............................................................................................
    {
    DbResult openStatus;
    DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(BE_SQLITE_OK == openStatus);
    ASSERT_TRUE(db.IsValid());

    //.........................................................................................
    TextAnnotation2dCPtr annotationElementC = TextAnnotation2d::Get(*db, insertedElementId);
    ASSERT_TRUE(annotationElementC.IsValid());

    // Spot check some properties; rely on other TextAnnotation tests to more fully test serialization, which should be relatively pass/fail on the element itself.
    TextAnnotationCP existingAnnotation = annotationElementC->GetAnnotation();
    ASSERT_TRUE(nullptr != existingAnnotation);
    EXPECT_TRUE(nullptr != existingAnnotation->GetFrameCP());
    EXPECT_TRUE(1 == existingAnnotation->GetLeaders().size());

    AnnotationTextBlockCP existingText = existingAnnotation->GetTextCP();
    ASSERT_TRUE(nullptr != existingText);
    EXPECT_TRUE(textStyleId == existingText->GetStyleId());

    AnnotationParagraphCollectionCR existingParagraphs = existingText->GetParagraphs();
    ASSERT_TRUE(1 == existingParagraphs.size());

    AnnotationRunCollectionCR existingRuns = existingParagraphs[0]->GetRuns();
    ASSERT_TRUE(1 == existingRuns.size());

    ASSERT_TRUE(AnnotationRunType::Text == existingRuns[0]->GetType());
    EXPECT_TRUE(0 == strcmp(ANNOTATION_TEXT_2, ((AnnotationTextRunCP) existingRuns[0].get())->GetContent().c_str()));
    }

    // Delete the element.
    //.............................................................................................
    {
    DbResult openStatus;
    DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(BE_SQLITE_OK == openStatus);
    ASSERT_TRUE(db.IsValid());

    //.........................................................................................
    TextAnnotation2dCPtr annotationElementC = TextAnnotation2d::Get(*db, insertedElementId);
    ASSERT_TRUE(annotationElementC.IsValid());

    annotationElementC->Delete();

    annotationElementC = TextAnnotation2d::Get(*db, insertedElementId);
    ASSERT_TRUE(!annotationElementC.IsValid());

    db->SaveChanges();
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
TEST_F(TextAnnotationTest, BasicCrud3d)
    {
    // The goal of this is to exercise persistence into and out of the database.
    // To defeat element caching, liberally open and close the DB.

    BeFileName dbPath;
    if (true)
        {
        SetupSeedProject();
        dbPath = BeFileName(m_db->GetDbFileName());
        }

    DgnModelId modelId;
    DgnElementId textStyleId;
    DgnElementId insertedElementId;
    static Utf8CP ANNOTATION_TEXT_1 = "Hello world.";
    static Utf8CP ANNOTATION_TEXT_2 = "Lorem ipsum dolar sit amet.";

    // Write the element to the database.
    //..........................................................................................
    {
    DbResult openStatus;
    DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(BE_SQLITE_OK == openStatus);
    ASSERT_TRUE(db.IsValid());

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*db, "Spatial Category");
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*db, TEST_NAME);
    modelId = model->GetModelId();

    textStyleId = ensureAnnotationTextStyle1(*db);
    ASSERT_TRUE(textStyleId.IsValid());

    TextAnnotation annotation(*db);
    annotation.SetText(AnnotationTextBlock::Create(*db, textStyleId, ANNOTATION_TEXT_1).get());
    ASSERT_TRUE(nullptr != annotation.GetTextCP());

    //.........................................................................................
    TextAnnotation3dPtr annotationElement = new TextAnnotation3d(TextAnnotation3d::CreateParams(*db, modelId, TextAnnotation3d::QueryDgnClassId(*db), categoryId));
    annotationElement->SetAnnotation(&annotation);
    ASSERT_TRUE(nullptr != annotationElement->GetAnnotation());

    TextAnnotation3dCPtr insertedAnnotationElement = annotationElement->Insert();
    ASSERT_TRUE(insertedAnnotationElement.IsValid());

    insertedElementId = insertedAnnotationElement->GetElementId();
    ASSERT_TRUE(insertedElementId.IsValid());

    // This is only here to aid in debugging so you can open the file in a viewer and see the element you just created.
    //.........................................................................................
        auto range = insertedAnnotationElement->CalculateRange3d();
        DgnDbTestUtils::InsertCameraView(*model, "TextAnnotation3dTest-BasicCrud", &range, StandardView::Top, Render::RenderMode::Wireframe);

        db->SaveChanges();
    }

    // Read the element back out, modify, and rewrite.
    //.............................................................................................
    {
    DbResult openStatus;
    DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(BE_SQLITE_OK == openStatus);
    ASSERT_TRUE(db.IsValid());

    //.........................................................................................
    TextAnnotation3dCPtr annotationElementC = TextAnnotation3d::Get(*db, insertedElementId);
    ASSERT_TRUE(annotationElementC.IsValid());

    // Spot check some properties; rely on other TextAnnotation tests to more fully test serialization, which should be relatively pass/fail on the element itself.
    TextAnnotationCP existingAnnotation = annotationElementC->GetAnnotation();
    ASSERT_TRUE(nullptr != existingAnnotation);
    EXPECT_TRUE(nullptr == existingAnnotation->GetFrameCP());
    EXPECT_TRUE(0 == existingAnnotation->GetLeaders().size());

    AnnotationTextBlockCP existingText = existingAnnotation->GetTextCP();
    ASSERT_TRUE(nullptr != existingText);
    EXPECT_TRUE(textStyleId == existingText->GetStyleId());

    AnnotationParagraphCollectionCR existingParagraphs = existingText->GetParagraphs();
    ASSERT_TRUE(1 == existingParagraphs.size());

    AnnotationRunCollectionCR existingRuns = existingParagraphs[0]->GetRuns();
    ASSERT_TRUE(1 == existingRuns.size());

    ASSERT_TRUE(AnnotationRunType::Text == existingRuns[0]->GetType());
    EXPECT_TRUE(0 == strcmp(ANNOTATION_TEXT_1, ((AnnotationTextRunCP) existingRuns[0].get())->GetContent().c_str()));

    // Update Annotation Element ( update text, and frame and leader )
    //.........................................................................................
    AnnotationFrameStylePtr frameStyle = AnnotationTestFixture::createAnnotationFrameStyle(*db, "TestFrameStyle");
    AnnotationFramePtr frame = AnnotationFrame::Create(*db, frameStyle->GetElementId());
    ASSERT_TRUE(frame.IsValid());

    AnnotationLeaderStylePtr leaderStyle = AnnotationTestFixture::createAnnotationLeaderStyle(*db, "TestLeaderStyle");
    AnnotationLeaderPtr leader = AnnotationLeader::Create(*db, leaderStyle->GetElementId());
    leader->SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType::PhysicalPoint);
    leader->SetSourceAttachmentType(AnnotationLeaderSourceAttachmentType::Id);
    ASSERT_TRUE(leader.IsValid());

    TextAnnotation annotation(*db);
    annotation.SetText(AnnotationTextBlock::Create(*db, textStyleId, ANNOTATION_TEXT_2).get());
    annotation.SetFrame(frame.get());
    ASSERT_TRUE(nullptr != annotation.GetFrameP());
    annotation.m_leaders.push_back(leader);

    TextAnnotation3dPtr annotationElement = TextAnnotation3d::GetForEdit(*db, insertedElementId);
    ASSERT_TRUE(annotationElement.IsValid());

    annotationElement->SetAnnotation(&annotation);

    TextAnnotation3dCPtr updatedAnnotationElement = annotationElement->Update();
    ASSERT_TRUE(updatedAnnotationElement.IsValid());
    EXPECT_TRUE(updatedAnnotationElement->GetElementId().IsValid());

    db->SaveChanges();
    }

    // Verify the modified element.
    //.........................................................................................
    {
    DbResult openStatus;
    DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(BE_SQLITE_OK == openStatus);
    ASSERT_TRUE(db.IsValid());

    //.........................................................................................
    TextAnnotation3dCPtr annotationElementC = TextAnnotation3d::Get(*db, insertedElementId);
    ASSERT_TRUE(annotationElementC.IsValid());

    // Spot check some properties; rely on other TextAnnotation tests to more fully test serialization, which should be relatively pass/fail on the element itself.
    TextAnnotationCP existingAnnotation = annotationElementC->GetAnnotation();
    ASSERT_TRUE(nullptr != existingAnnotation);
    EXPECT_TRUE(nullptr != existingAnnotation->GetFrameCP());
    EXPECT_TRUE(1 == existingAnnotation->GetLeaders().size());

    AnnotationTextBlockCP existingText = existingAnnotation->GetTextCP();
    ASSERT_TRUE(nullptr != existingText);
    EXPECT_TRUE(textStyleId == existingText->GetStyleId());

    AnnotationParagraphCollectionCR existingParagraphs = existingText->GetParagraphs();
    ASSERT_TRUE(1 == existingParagraphs.size());

    AnnotationRunCollectionCR existingRuns = existingParagraphs[0]->GetRuns();
    ASSERT_TRUE(1 == existingRuns.size());

    ASSERT_TRUE(AnnotationRunType::Text == existingRuns[0]->GetType());
    EXPECT_TRUE(0 == strcmp(ANNOTATION_TEXT_2, ((AnnotationTextRunCP) existingRuns[0].get())->GetContent().c_str()));
    }

    // Delete the element.
    //..........................................................................................
    {
    DbResult openStatus;
    DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(BE_SQLITE_OK == openStatus);
    ASSERT_TRUE(db.IsValid());

    //.........................................................................................
    TextAnnotation3dCPtr annotationElementC = TextAnnotation3d::Get(*db, insertedElementId);
    ASSERT_TRUE(annotationElementC.IsValid());

    annotationElementC->Delete();

    annotationElementC = TextAnnotation3d::Get(*db, insertedElementId);
    ASSERT_TRUE(!annotationElementC.IsValid());

    db->SaveChanges();
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
// TEST_F(TextAnnotationTest, CreateTextWithFramesAndLeaders)
//     {
//     SetupSeedProject();
//     BeFileName dbPath = BeFileName(m_db->GetDbFileName());

//     DbResult openStatus;
//     DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));

//     DgnElementId textStyleId;
//         {
//         AnnotationTextStyle textStyle(*db);
//             textStyle.SetColorType(AnnotationColorType::RGBA);
//             textStyle.SetColorValue(ColorDef(0xff, 0xff, 0xff));
//             textStyle.SetFontId(db->Fonts().AcquireId(DgnFontManager::GetLastResortTrueTypeFont()));
//             textStyle.SetHeight(1000.0);
//             textStyle.SetName("Text Style 1");
        
//         textStyleId = textStyle.Insert()->GetElementId();
//         }

//     DgnElementId frameBoxStyleId;
//         {
//         AnnotationFrameStyle frameStyle(*db);
//             frameStyle.SetFillColorType(AnnotationColorType::RGBA);
//             frameStyle.SetFillColorValue(ColorDef(0x33, 0x33, 0x33));
//             frameStyle.SetIsFillEnabled(true);
//             frameStyle.SetIsStrokeEnabled(true);
//             frameStyle.SetName("Frame Box Style 1");
//             frameStyle.SetPadding(1.5);
//             frameStyle.SetStrokeColorType(AnnotationColorType::RGBA);
//             frameStyle.SetStrokeColorValue(ColorDef(0x00, 0xff, 0xff));
//             frameStyle.SetStrokeWeight(1);
//             frameStyle.SetType(AnnotationFrameType::Box);
        
//         frameBoxStyleId = frameStyle.Insert()->GetElementId();
//         }

//     DgnElementId frameCloudStyleId;
//         {
//         AnnotationFrameStyle frameStyle(*db);
//             frameStyle.SetCloudBulgeFactor(1.25);
//             frameStyle.SetCloudDiameterFactor(1.5);
//             frameStyle.SetFillColorType(AnnotationColorType::RGBA);
//             frameStyle.SetFillColorValue(ColorDef(0x33, 0x33, 0x33));
//             frameStyle.SetIsFillEnabled(true);
//             frameStyle.SetIsStrokeCloud(true);
//             frameStyle.SetIsStrokeEnabled(true);
//             frameStyle.SetName("Frame Cloud Style 1");
//             frameStyle.SetPadding(1.5);
//             frameStyle.SetStrokeColorType(AnnotationColorType::RGBA);
//             frameStyle.SetStrokeColorValue(ColorDef(0x00, 0xff, 0xff));
//             frameStyle.SetStrokeWeight(1);
//             frameStyle.SetType(AnnotationFrameType::Box);
        
//         frameCloudStyleId = frameStyle.Insert()->GetElementId();
//         }

//     DgnElementId frameEllipseStyleId;
//         {
//         AnnotationFrameStyle frameStyle(*db);
//             frameStyle.SetFillColorType(AnnotationColorType::RGBA);
//             frameStyle.SetFillColorValue(ColorDef(0x33, 0x33, 0x33));
//             frameStyle.SetIsFillEnabled(true);
//             frameStyle.SetIsStrokeEnabled(true);
//             frameStyle.SetName("Frame Ellipse Style 1");
//             frameStyle.SetPadding(1.5);
//             frameStyle.SetStrokeColorType(AnnotationColorType::RGBA);
//             frameStyle.SetStrokeColorValue(ColorDef(0x00, 0xff, 0xff));
//             frameStyle.SetStrokeWeight(1);
//             frameStyle.SetType(AnnotationFrameType::Ellipse);
        
//         frameEllipseStyleId = frameStyle.Insert()->GetElementId();
//         }

//     DgnElementId leaderStyleId;
//         {
//         AnnotationLeaderStyle leaderStyle(*db);
//             leaderStyle.SetLineColorType(AnnotationColorType::RGBA);
//             leaderStyle.SetLineColorValue(ColorDef(0xff, 0x00, 0x00));
//             leaderStyle.SetLineType(AnnotationLeaderLineType::Straight);
//             leaderStyle.SetLineWeight(1);
//             leaderStyle.SetName("Leader Style 1");
//             leaderStyle.SetTerminatorColorType(AnnotationColorType::RGBA);
//             leaderStyle.SetTerminatorColorValue(ColorDef(0xff, 0x00, 0x00));
//             leaderStyle.SetTerminatorScaleFactor(1.0);
//             leaderStyle.SetTerminatorType(AnnotationLeaderTerminatorType::ClosedArrow);
//             leaderStyle.SetTerminatorWeight(1);
        
//         leaderStyleId = leaderStyle.Insert()->GetElementId();
//         }

//     DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*db, "Spatial Category");
//     PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*db, "Text Annotation Elements");
//     DgnModelId modelId = model->GetModelId();

//     //.........................................................................................
//     AnnotationTextBlockPtr annotationText1 = AnnotationTextBlock::Create(*db, textStyleId, "The magician got so mad, he pulled his hare out!");
//     AnnotationFramePtr annotationFrame1 = AnnotationFrame::Create(*db, frameBoxStyleId);
//     TextAnnotation annotation1(*db);
//         annotation1.SetFrame(annotationFrame1.get());
//         annotation1.SetText(annotationText1.get());

//     TextAnnotation3dPtr annotationElement1 = new TextAnnotation3d(TextAnnotation3d::CreateParams(*db, modelId, TextAnnotation3d::QueryDgnClassId(*db), categoryId));
//         annotationElement1->SetAnnotation(&annotation1);

//     TextAnnotation3dCPtr insertedAnnotationElement1 = annotationElement1->Insert();

//     //.........................................................................................
//     AnnotationTextBlockPtr annotationText2 = AnnotationTextBlock::Create(*db, textStyleId);
//         annotationText2->AppendRun(*AnnotationTextRun::Create(*db, textStyleId, "Office: Sir, I stopped you for speeding. Do you know how fast you were going?"));
//         annotationText2->AppendParagraph();
//         annotationText2->AppendRun(*AnnotationTextRun::Create(*db, textStyleId, "Driver: I was trying to keep up with the traffic."));
//         annotationText2->AppendParagraph();
//         annotationText2->AppendRun(*AnnotationTextRun::Create(*db, textStyleId, "Officer: But there is no traffic."));
//         annotationText2->AppendParagraph();
//         annotationText2->AppendRun(*AnnotationTextRun::Create(*db, textStyleId, "Driver: That's how far behind I am."));
//     AnnotationFramePtr annotationFrame2 = AnnotationFrame::Create(*db, frameCloudStyleId);
//     TextAnnotation annotation2(*db);
//         annotation2.SetFrame(annotationFrame2.get());
//         annotation2.SetText(annotationText2.get());

//     TextAnnotation3dPtr annotationElement2 = new TextAnnotation3d(TextAnnotation3d::CreateParams(*db, modelId, TextAnnotation3d::QueryDgnClassId(*db), categoryId));
//         annotationElement2->SetPlacement(Placement3d(DPoint3d::From(0.0, -8000.0, 0.0), YawPitchRollAngles()));
//         annotationElement2->SetAnnotation(&annotation2);

//     TextAnnotation3dCPtr insertedAnnotationElement2 = annotationElement2->Insert();

//     //.........................................................................................
//     AnnotationTextBlockPtr annotationText3 = AnnotationTextBlock::Create(*db, textStyleId, "My friend David was a victim if ID theft. Now we just call him Dav.");
//     AnnotationFramePtr annotationFrame3 = AnnotationFrame::Create(*db, frameEllipseStyleId);
//     AnnotationLeaderPtr annotationLeader3A = AnnotationLeader::Create(*db, leaderStyleId);
//         annotationLeader3A->SetSourceAttachmentType(AnnotationLeaderSourceAttachmentType::Id);
//         annotationLeader3A->SetSourceAttachmentDataForId(0);
//         annotationLeader3A->SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType::PhysicalPoint);
//         annotationLeader3A->SetTargetAttachmentDataForPhysicalPoint(DPoint3d::From(60000.0, 15000.0, 0.0));
//     AnnotationLeaderPtr annotationLeader3B = AnnotationLeader::Create(*db, leaderStyleId);
//         annotationLeader3B->SetSourceAttachmentType(AnnotationLeaderSourceAttachmentType::Id);
//         annotationLeader3B->SetSourceAttachmentDataForId(1);
//         annotationLeader3B->SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType::PhysicalPoint);
//         annotationLeader3B->SetTargetAttachmentDataForPhysicalPoint(DPoint3d::From(30000.0, 20000.0, 0.0));
//     TextAnnotation annotation3(*db);
//         annotation3.SetFrame(annotationFrame3.get());
//         annotation3.GetLeadersR().push_back(annotationLeader3A);
//         annotation3.GetLeadersR().push_back(annotationLeader3B);
//         annotation3.SetText(annotationText3.get());

//     TextAnnotation3dPtr annotationElement3 = new TextAnnotation3d(TextAnnotation3d::CreateParams(*db, modelId, TextAnnotation3d::QueryDgnClassId(*db), categoryId));
//         annotationElement3->SetPlacement(Placement3d(DPoint3d::From(0.0, 8000.0, 0.0), YawPitchRollAngles()));
//         annotationElement3->SetAnnotation(&annotation3);

//     TextAnnotation3dCPtr insertedAnnotationElement3 = annotationElement3->Insert();

//     //.........................................................................................
//     AxisAlignedBox3d range = insertedAnnotationElement1->CalculateRange3d();
//     range.Extend(insertedAnnotationElement2->CalculateRange3d());
//     range.Extend(insertedAnnotationElement3->CalculateRange3d());
    
//     DgnDbTestUtils::InsertCameraView(*model, "View of Text", &range, StandardView::Top, Render::RenderMode::SmoothShade);

//     range.ScaleAboutCenter(range, 2.0);
//     db->GeoLocation().SetProjectExtents(range);

//     //.........................................................................................
//     db->SaveChanges();
//     }
