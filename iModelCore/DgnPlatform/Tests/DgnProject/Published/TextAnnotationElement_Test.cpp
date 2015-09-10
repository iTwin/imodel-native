//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/TextAnnotationElement_Test.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatform/DgnCore/Annotations/TextAnnotationElement.h>

USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
static DgnStyleId ensureAnnotationTextStyle1(DgnDbR db)
    {
    static const CharCP STYLE_NAME = "AnnotationTextStyle1";
    AnnotationTextStylePtr existingStyle = db.Styles().AnnotationTextStyles().QueryByName(STYLE_NAME);
    if (existingStyle.IsValid())
        return existingStyle->GetId();
    
    AnnotationTextStyle style(db);
    style.SetColor(ElementColor(ColorDef(0x00, 0xff, 0x00)));
    style.SetFontId(db.Fonts().AcquireId(DgnFontManager::GetAnyLastResortFont()));
    style.SetHeight(1000.0);
    style.SetName(STYLE_NAME);

    db.Styles().AnnotationTextStyles().Insert(style);
    
    return style.GetId();
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
    BeTest::GetHost().GetOutputRoot(dbPath);
    dbPath.AppendToPath(L"TextAnnotationElementTest-BasicCrud.idgndb"); // use .idgndb so that sample navigator easily opens it, even though .dgndb would be more appropriate

    DgnCategoryId categoryId;
    DgnModelId modelId;
    DgnStyleId textStyleId;
    DgnElementId element1Id;
    static Utf8CP ANNOTATION_TEXT_1 = "Hello world.";
    static Utf8CP ANNOTATION_TEXT_2 = "Lorem ipsum dolar sit amet.";

    // Write the element to the database.
    //.............................................................................................
        {
        CreateDgnDbParams dbCreateParams;
        dbCreateParams.SetOverwriteExisting(true);
        dbCreateParams.SetProjectName("TextAnnotationElementTest-BasicCrud");
        dbCreateParams.SetProjectDescription("Created by unit test TextAnnotationElementTest.BasicCrud");
        dbCreateParams.SetStartDefaultTxn(DefaultTxn::Exclusive);

        DbResult createStatus;
        DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, dbPath, dbCreateParams);
        ASSERT_TRUE(BE_SQLITE_OK == createStatus);
        ASSERT_TRUE(db.IsValid());
        
        DgnCategories::Category category("Annotation Category 1", DgnCategories::Scope::Annotation);
        DgnCategories::SubCategory::Appearance categoryAppearance;
        ASSERT_TRUE(BE_SQLITE_OK == db->Categories().Insert(category, categoryAppearance));

        categoryId = category.GetCategoryId();
        ASSERT_TRUE(categoryId.IsValid());

        DgnModelPtr model = new GraphicsModel2d(DgnModel::CreateParams(*db, DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_GraphicsModel2d)), "2D Model 1"));
        ASSERT_TRUE(DgnDbStatus::Success == model->Insert());

        modelId = model->GetModelId();
        ASSERT_TRUE(modelId.IsValid());

        textStyleId = ensureAnnotationTextStyle1(*db);
        
        TextAnnotation annotation(*db);
        annotation.SetText(AnnotationTextBlock::Create(*db, textStyleId, ANNOTATION_TEXT_1).get());

        TextAnnotationElementPtr annotationElement1 = new TextAnnotationElement(TextAnnotationElement::CreateParams(*db, modelId, TextAnnotationElement::QueryDgnClassId(*db), categoryId));
        annotationElement1->SetAnnotation(&annotation);
        TextAnnotationElementCPtr insertedAnnotationElement1 = annotationElement1->Insert();
        ASSERT_TRUE(insertedAnnotationElement1.IsValid());
        
        element1Id = insertedAnnotationElement1->GetElementId();
        ASSERT_TRUE(element1Id.IsValid());

        // This is only here to aid in debugging so you can open the file in a viewer and see the element you just created.
        //.............................................................................................
        DgnViews::View view;
        view.SetDgnViewType(DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "DrawingView")), DgnViewType::Drawing);
        view.SetDgnViewSource(DgnViewSource::Generated);
        view.SetName("TextAnnotationElementTest-BasicCrud");
        view.SetBaseModelId(modelId);

        EXPECT_TRUE(BE_SQLITE_OK == db->Views().Insert(view));
        EXPECT_TRUE(view.GetId().IsValid());

        ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);
        
        DrawingViewController viewController(*db, view.GetId());
        viewController.SetStandardViewRotation(StandardView::Top);
        viewController.LookAtVolume(insertedAnnotationElement1->CalculateRange3d(), nullptr, &viewMargin);
        viewController.GetViewFlagsR().SetRenderMode(DgnRenderMode::Wireframe);
        viewController.ChangeCategoryDisplay(categoryId, true);
        viewController.ChangeModelDisplay(modelId, true);

        EXPECT_TRUE(BE_SQLITE_OK == viewController.Save());
        db->SaveSettings();
        }

    // Read the element back out, modify, and rewrite.
    //.............................................................................................
        {
        DbResult openStatus;
        DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(BE_SQLITE_OK == openStatus);
        ASSERT_TRUE(db.IsValid());
        
        TextAnnotationElementCPtr annotationElementC = TextAnnotationElement::Get(*db, element1Id);
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
        TextAnnotation annotation(*db);
        annotation.SetText(AnnotationTextBlock::Create(*db, textStyleId, ANNOTATION_TEXT_2).get());

        TextAnnotationElementPtr annotationElement = TextAnnotationElement::GetForEdit(*db, element1Id);
        ASSERT_TRUE(annotationElement.IsValid());

        annotationElement->SetAnnotation(&annotation);
        
        TextAnnotationElementCPtr updatedAnnotationElement = annotationElement->Update();
        ASSERT_TRUE(updatedAnnotationElement.IsValid());
        EXPECT_TRUE(updatedAnnotationElement->GetElementId().IsValid());
        }
    
    // Verify the modified element.
    //.............................................................................................
        {
        DbResult openStatus;
        DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        ASSERT_TRUE(BE_SQLITE_OK == openStatus);
        ASSERT_TRUE(db.IsValid());

        TextAnnotationElementCPtr annotationElementC = TextAnnotationElement::Get(*db, element1Id);
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
    }
