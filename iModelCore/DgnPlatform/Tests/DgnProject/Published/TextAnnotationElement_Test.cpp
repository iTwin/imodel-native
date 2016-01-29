//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/TextAnnotationElement_Test.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"
#include <DgnPlatform/Annotations/Annotations.h>
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
TEST(TextAnnotationElementTest, BasicCrud)
    {
    // The goal of this is to exercise persistence into and out of the database.
    // To defeat element caching, liberally open and close the DB.
    
    ScopedDgnHost host;

    BeFileName dbPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(dbPath, L"2dMetricGeneral.idgndb", L"TextAnnotationElementTest-BasicCrud.dgndb", __FILE__));

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

        DgnModelPtr model = new GeometricModel2d(GeometricModel2d::CreateParams(*db, DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_GeometricModel2d)), DgnModel::CreateModelCode("2D Model")));
        ASSERT_TRUE(DgnDbStatus::Success == model->Insert());

        modelId = model->GetModelId();
        ASSERT_TRUE(modelId.IsValid());

        textStyleId = ensureAnnotationTextStyle1(*db);
        ASSERT_TRUE(textStyleId.IsValid());

        TextAnnotation annotation(*db);
        annotation.SetText(AnnotationTextBlock::Create(*db, textStyleId, ANNOTATION_TEXT_1).get());
        ASSERT_TRUE(nullptr != annotation.GetTextCP());

        //.........................................................................................
        TextAnnotationElementPtr annotationElement = new TextAnnotationElement(TextAnnotationElement::CreateParams(*db, modelId, TextAnnotationElement::QueryDgnClassId(*db), categoryId));
        annotationElement->SetAnnotation(&annotation);
        ASSERT_TRUE(nullptr != annotationElement->GetAnnotation());

        TextAnnotationElementCPtr insertedAnnotationElement = annotationElement->Insert();
        ASSERT_TRUE(insertedAnnotationElement.IsValid());
        
        insertedElementId = insertedAnnotationElement->GetElementId();
        ASSERT_TRUE(insertedElementId.IsValid());

        // This is only here to aid in debugging so you can open the file in a viewer and see the element you just created.
        //.........................................................................................
        DrawingViewDefinition view(DrawingViewDefinition::CreateParams(*db, "TextAnnotationElementTest-BasicCrud",
                    ViewDefinition::Data(modelId, DgnViewSource::Generated)));
        EXPECT_TRUE(view.Insert().IsValid());

        ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);
        
        DrawingViewController viewController(*db, view.GetViewId());
        viewController.SetStandardViewRotation(StandardView::Top);
        viewController.LookAtVolume(insertedAnnotationElement->CalculateRange3d(), nullptr, &viewMargin);
        viewController.GetViewFlagsR().SetRenderMode(DgnRenderMode::Wireframe);
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
        TextAnnotationElementCPtr annotationElementC = TextAnnotationElement::Get(*db, insertedElementId);
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
        
        // Update with different text.
        //.........................................................................................
        TextAnnotation annotation(*db);
        annotation.SetText(AnnotationTextBlock::Create(*db, textStyleId, ANNOTATION_TEXT_2).get());

        TextAnnotationElementPtr annotationElement = TextAnnotationElement::GetForEdit(*db, insertedElementId);
        ASSERT_TRUE(annotationElement.IsValid());

        annotationElement->SetAnnotation(&annotation);
        
        TextAnnotationElementCPtr updatedAnnotationElement = annotationElement->Update();
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
        TextAnnotationElementCPtr annotationElementC = TextAnnotationElement::Get(*db, insertedElementId);
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
        TextAnnotationElementCPtr annotationElementC = TextAnnotationElement::Get(*db, insertedElementId);
        ASSERT_TRUE(annotationElementC.IsValid());

        annotationElementC->Delete();

        annotationElementC = TextAnnotationElement::Get(*db, insertedElementId);
        ASSERT_TRUE(!annotationElementC.IsValid());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
TEST(SpatialTextAnnotationElementTest, BasicCrud)
    {
    // The goal of this is to exercise persistence into and out of the database.
    // To defeat element caching, liberally open and close the DB.
    
    ScopedDgnHost host;

    BeFileName dbPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(dbPath, L"3dMetricGeneral.idgndb", L"SpatialTextAnnotationElementTest-BasicCrud.dgndb", __FILE__));

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

        DgnModelPtr model = new SpatialModel(SpatialModel::CreateParams(*db, DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel)), DgnModel::CreateModelCode("Physical Model")));
        ASSERT_TRUE(DgnDbStatus::Success == model->Insert());

        modelId = model->GetModelId();
        ASSERT_TRUE(modelId.IsValid());

        textStyleId = ensureAnnotationTextStyle1(*db);
        ASSERT_TRUE(textStyleId.IsValid());

        TextAnnotation annotation(*db);
        annotation.SetText(AnnotationTextBlock::Create(*db, textStyleId, ANNOTATION_TEXT_1).get());
        ASSERT_TRUE(nullptr != annotation.GetTextCP());

        //.........................................................................................
        SpatialTextAnnotationElementPtr annotationElement = new SpatialTextAnnotationElement(SpatialTextAnnotationElement::CreateParams(*db, modelId, SpatialTextAnnotationElement::QueryDgnClassId(*db), categoryId));
        annotationElement->SetAnnotation(&annotation);
        ASSERT_TRUE(nullptr != annotationElement->GetAnnotation());

        SpatialTextAnnotationElementCPtr insertedAnnotationElement = annotationElement->Insert();
        ASSERT_TRUE(insertedAnnotationElement.IsValid());
        
        insertedElementId = insertedAnnotationElement->GetElementId();
        ASSERT_TRUE(insertedElementId.IsValid());

        // This is only here to aid in debugging so you can open the file in a viewer and see the element you just created.
        //.........................................................................................
        CameraViewDefinition view(CameraViewDefinition::CreateParams(*db, "SpatialTextAnnotationElementTest-BasicCrud",
                    ViewDefinition::Data(modelId, DgnViewSource::Generated)));
        EXPECT_TRUE(view.Insert().IsValid());

        ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);
        
        SpatialViewController viewController(*db, view.GetViewId());
        viewController.SetStandardViewRotation(StandardView::Top);
        viewController.LookAtVolume(insertedAnnotationElement->CalculateRange3d(), nullptr, &viewMargin);
        viewController.GetViewFlagsR().SetRenderMode(DgnRenderMode::Wireframe);
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
        SpatialTextAnnotationElementCPtr annotationElementC = SpatialTextAnnotationElement::Get(*db, insertedElementId);
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
        
        // Update with different text.
        //.........................................................................................
        TextAnnotation annotation(*db);
        annotation.SetText(AnnotationTextBlock::Create(*db, textStyleId, ANNOTATION_TEXT_2).get());

        SpatialTextAnnotationElementPtr annotationElement = SpatialTextAnnotationElement::GetForEdit(*db, insertedElementId);
        ASSERT_TRUE(annotationElement.IsValid());

        annotationElement->SetAnnotation(&annotation);
        
        SpatialTextAnnotationElementCPtr updatedAnnotationElement = annotationElement->Update();
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
        SpatialTextAnnotationElementCPtr annotationElementC = SpatialTextAnnotationElement::Get(*db, insertedElementId);
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
        SpatialTextAnnotationElementCPtr annotationElementC = SpatialTextAnnotationElement::Get(*db, insertedElementId);
        ASSERT_TRUE(annotationElementC.IsValid());

        annotationElementC->Delete();

        annotationElementC = SpatialTextAnnotationElement::Get(*db, insertedElementId);
        ASSERT_TRUE(!annotationElementC.IsValid());
        }
    }
