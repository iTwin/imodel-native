//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/TextAnnotationElement_Test.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"
#include <DgnPlatform/Annotations/TextAnnotationElement.h>

USING_NAMESPACE_BENTLEY_SQLITE

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
TEST(TextAnnotation2dTest, BasicCrud)
    {
    // The goal of this is to exercise persistence into and out of the database.
    // To defeat element caching, liberally open and close the DB.
    
    ScopedDgnHost host;

    BeFileName dbPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(dbPath, L"2dMetricGeneral.ibim", L"TextAnnotation2dTest-BasicCrud.bim", __FILE__));

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

        DgnModelPtr model = new GeometricModel2d(GeometricModel2d::CreateParams(*db, DgnClassId(db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, DGN_CLASSNAME_GeometricModel2d)), DgnModel::CreateModelCode("2D Model")));
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
        DrawingViewDefinition view(DrawingViewDefinition::CreateParams(*db, "TextAnnotation2dTest-BasicCrud",
                    ViewDefinition::Data(modelId, DgnViewSource::Generated)));
        EXPECT_TRUE(view.Insert().IsValid());

        ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);
        
        DrawingViewController viewController(*db, view.GetViewId());
        viewController.SetStandardViewRotation(StandardView::Top);
        viewController.LookAtVolume(insertedAnnotationElement->CalculateRange3d(), nullptr, &viewMargin);
        viewController.GetViewFlagsR().SetRenderMode(Render::RenderMode::Wireframe);
        viewController.ChangeCategoryDisplay(categoryId, true);
        viewController.ChangeModelDisplay(modelId, true);

        EXPECT_TRUE(BE_SQLITE_OK == viewController.Save());
        db->SaveSettings();
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
TEST(TextAnnotation3dTest, BasicCrud)
    {
    // The goal of this is to exercise persistence into and out of the database.
    // To defeat element caching, liberally open and close the DB.
    
    ScopedDgnHost host;

    BeFileName dbPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(dbPath, L"3dMetricGeneral.ibim", L"TextAnnotation3dTest-BasicCrud.bim", __FILE__));

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

        DgnModelPtr model = new SpatialModel(SpatialModel::CreateParams(*db, DgnClassId(db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel)), DgnModel::CreateModelCode("Physical Model")));
        ASSERT_TRUE(DgnDbStatus::Success == model->Insert());

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
        CameraViewDefinition view(CameraViewDefinition::CreateParams(*db, "TextAnnotation3dTest-BasicCrud",
                    ViewDefinition::Data(modelId, DgnViewSource::Generated)));
        EXPECT_TRUE(view.Insert().IsValid());

        ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);
        
        SpatialViewController viewController(*db, view.GetViewId());
        viewController.SetStandardViewRotation(StandardView::Top);
        viewController.LookAtVolume(insertedAnnotationElement->CalculateRange3d(), nullptr, &viewMargin);
        viewController.GetViewFlagsR().SetRenderMode(Render::RenderMode::Wireframe);
        viewController.ChangeCategoryDisplay(categoryId, true);
        viewController.ChangeModelDisplay(modelId, true);

        EXPECT_TRUE(BE_SQLITE_OK == viewController.Save());
        db->SaveSettings();
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
