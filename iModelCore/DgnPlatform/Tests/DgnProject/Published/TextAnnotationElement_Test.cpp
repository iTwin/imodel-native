//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/TextAnnotationElement_Test.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"
#include <DgnPlatform/Annotations/TextAnnotationElement.h>

USING_NAMESPACE_BENTLEY_SQLITE
/*---------------------------------------------------------------------------------**//**
* @bsistruct                                            Umar.Hayat      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct TextAnnotationTest : public ::testing::Test
{
private:
    ScopedDgnHost host;
};
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
static DgnElementId ensureAnnotationTextStyle1(DgnDbR db)
    {
    static const Utf8CP STYLE_NAME = "AnnotationTextStyle1";
    AnnotationTextStyleCPtr existingStyle = AnnotationTextStyle::Get(db, STYLE_NAME);
    if (existingStyle.IsValid())
        return existingStyle->GetElementId();
    
    AnnotationTextStyle style(db);
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
TEST_F (TextAnnotationTest, BasicCrud2d)
    {
    // The goal of this is to exercise persistence into and out of the database.
    // To defeat element caching, liberally open and close the DB.

    BeFileName fileName(TEST_FIXTURE_NAME, true);
    fileName.AppendToPath(L"TextAnnotation2dTest-BasicCrud.dgndb");

    BeFileName dbPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(dbPath, L"2dMetricGeneral.idgndb", fileName.c_str(), __FILE__));

    DgnModelId modelId;
    DgnElementId textStyleId;
    DgnElementId insertedElementId;
    static Utf8CP ANNOTATION_TEXT_1 = "Hello world.";
    static Utf8CP ANNOTATION_TEXT_2 = "Lorem ipsum dolar sit amet.";

    // Write the element to the database.
    //.............................................................................................
        {
        //.........................................................................................
        DbResult openStatus;
        DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(BE_SQLITE_OK == openStatus);
        ASSERT_TRUE(db.IsValid());
        
        DgnCategory category(DgnCategory::CreateParams(*db, "Annotation Category", DgnCategory::Scope::Annotation));
        DgnSubCategory::Appearance categoryAppearance;
        category.Insert(categoryAppearance);

        DgnCategoryId categoryId = category.GetCategoryId();
        ASSERT_TRUE(categoryId.IsValid());

        RefCountedPtr<DrawingModel> model = new DrawingModel(DrawingModel::CreateParams(*db, DgnClassId(db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingModel)), DgnElementId() /* WIP: Which element? */, DgnModel::CreateModelCode("2D Model")));
        ASSERT_TRUE(DgnDbStatus::Success == model->Insert());

        modelId = model->GetModelId();
        ASSERT_TRUE(modelId.IsValid());

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
        auto viewDef = DrawingViewDefinition::MakeViewOfModel(*model, "TextAnnotation2dTest-BasicCrud");
        EXPECT_TRUE(viewDef->Insert().IsValid());
        }

    // Read the element back out, modify, and rewrite.
    //.............................................................................................
        {
        //.........................................................................................
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
        EXPECT_TRUE(0 == strcmp(ANNOTATION_TEXT_1, ((AnnotationTextRunCP)existingRuns[0].get())->GetContent().c_str()));

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
        }
    
    // Verify the modified element.
    //.............................................................................................
        {
        //.........................................................................................
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
        EXPECT_TRUE(0 == strcmp(ANNOTATION_TEXT_2, ((AnnotationTextRunCP)existingRuns[0].get())->GetContent().c_str()));
        }
    
    // Delete the element.
    //.............................................................................................
        {
        //.........................................................................................
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
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
TEST_F (TextAnnotationTest, BasicCrud3d)
    {
    // The goal of this is to exercise persistence into and out of the database.
    // To defeat element caching, liberally open and close the DB.

    BeFileName fileName(TEST_FIXTURE_NAME, true);
    fileName.AppendToPath(L"TextAnnotation3dTest-BasicCrud.dgndb");
    
    BeFileName dbPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(dbPath, L"3dMetricGeneral.idgndb", fileName.c_str(), __FILE__));

    DgnModelId modelId;
    DgnElementId textStyleId;
    DgnElementId insertedElementId;
    static Utf8CP ANNOTATION_TEXT_1 = "Hello world.";
    static Utf8CP ANNOTATION_TEXT_2 = "Lorem ipsum dolar sit amet.";

    // Write the element to the database.
    //.............................................................................................
        {
        //.........................................................................................
        DbResult openStatus;
        DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(BE_SQLITE_OK == openStatus);
        ASSERT_TRUE(db.IsValid());
        
        DgnCategory category(DgnCategory::CreateParams(*db, "Physical Category", DgnCategory::Scope::Physical));
        DgnSubCategory::Appearance categoryAppearance;
        category.Insert(categoryAppearance);

        DgnCategoryId categoryId = category.GetCategoryId();
        ASSERT_TRUE(categoryId.IsValid());

        SubjectCPtr rootSubject = db->Elements().GetRootSubject();
        SubjectCPtr modelSubject = Subject::CreateAndInsert(*rootSubject, TEST_NAME); // create a placeholder Subject for the DgnModel to describe
        ASSERT_TRUE(modelSubject.IsValid());
        PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*modelSubject, DgnModel::CreateModelCode(TEST_NAME));
        ASSERT_TRUE(model.IsValid());
        modelId = model->GetModelId();
        ASSERT_TRUE(modelId.IsValid());

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
        }

    // Read the element back out, modify, and rewrite.
    //.............................................................................................
        {
        //.........................................................................................
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
        EXPECT_TRUE(0 == strcmp(ANNOTATION_TEXT_1, ((AnnotationTextRunCP)existingRuns[0].get())->GetContent().c_str()));
        
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
        }
    
    // Verify the modified element.
    //.............................................................................................
        {
        //.........................................................................................
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
        EXPECT_TRUE(0 == strcmp(ANNOTATION_TEXT_2, ((AnnotationTextRunCP)existingRuns[0].get())->GetContent().c_str()));
        }
    
    // Delete the element.
    //.............................................................................................
        {
        //.........................................................................................
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
        }
    }
