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

    DgnCategoryId physicalCategory1Id;
    DgnModelId physicalModel1Id;
    DgnStyleId annotationTextStyle1Id;
    DgnElementId annotationElement1Id;
    static Utf8CP ANNOTATION_TEXT_1 = "Hello world.";
    static Utf8CP ANNOTATION_TEXT_2 = "Lorem ipsum dolar sit amet.";

    // Write the element to the database.
    //.............................................................................................
        {
        CreateDgnDbParams dbCreateParams;
        dbCreateParams.SetProjectName("TextAnnotationElementTest-BasicCrud");
        dbCreateParams.SetProjectDescription("Created by unit test TextAnnotationElementTest.BasicCrud");
        dbCreateParams.SetStartDefaultTxn(DefaultTxn_Exclusive);

        DbResult createStatus;
        DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, dbPath, dbCreateParams);
        ASSERT_TRUE(DGNFILE_STATUS_Success == createStatus);
        ASSERT_TRUE(db.IsValid());
        
        DgnCategories::Category physicalCategory1("Physical Category 1", DgnCategories::Scope::Physical);
        DgnCategories::SubCategory::Appearance physicalCategory1Appearance;
        ASSERT_TRUE(BE_SQLITE_OK == db->Categories().Insert(physicalCategory1, physicalCategory1Appearance));

        physicalCategory1Id = physicalCategory1.GetCategoryId();
        ASSERT_TRUE(physicalCategory1Id.IsValid());

        DgnModelPtr physicalModel1 = new PhysicalModel(DgnModel::CreateParams(*db, DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel)), "Physical Model 1"));
        ASSERT_TRUE(DGNMODEL_STATUS_Success == db->Models().Insert(*physicalModel1));

        physicalModel1Id = physicalModel1->GetModelId();
        ASSERT_TRUE(physicalModel1Id.IsValid());

        annotationTextStyle1Id = ensureAnnotationTextStyle1(*db);
        
        TextAnnotation annotation(*db);
        annotation.SetText(AnnotationTextBlock::Create(*db, annotationTextStyle1Id, ANNOTATION_TEXT_1).get());

        PhysicalTextAnnotationElementPtr annotationElement1 = PhysicalTextAnnotationElement::Create(PhysicalTextAnnotationElement::CreateParams(*physicalModel1, PhysicalTextAnnotationElement::QueryClassId(*db), physicalCategory1Id), annotation);
        PhysicalTextAnnotationElementCPtr insertedAnnotationElement1 = annotationElement1->Insert();
        ASSERT_TRUE(insertedAnnotationElement1.IsValid());
        
        annotationElement1Id = insertedAnnotationElement1->GetElementId();
        ASSERT_TRUE(annotationElement1Id.IsValid());

        // This is only here to aid in debugging so you can open the file in a viewer and see the element you just created.
        //.............................................................................................
        DgnViews::View view;
        view.SetDgnViewType(DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalView)), DgnViewType::Physical);
        view.SetDgnViewSource(DgnViewSource::Generated);
        view.SetName("TextAnnotationElementTest-BasicCrud");
        view.SetBaseModelId(physicalModel1Id);

        EXPECT_TRUE(BE_SQLITE_OK == db->Views().Insert(view));
        EXPECT_TRUE(view.GetId().IsValid());

        ViewController::MarginPercent viewMargin(0.1, 0.1, 0.1, 0.1);
        
        PhysicalViewController viewController(*db, view.GetId());
        viewController.SetStandardViewRotation(StandardView::Top);
        viewController.LookAtVolume(insertedAnnotationElement1->GetPlacement().GetElementBox()/*, nullptr, &viewMargin*/);
        viewController.GetViewFlagsR().SetRenderMode(DgnRenderMode::Wireframe);
        viewController.ChangeCategoryDisplay(physicalCategory1Id, true);
        viewController.ChangeModelDisplay(physicalModel1Id, true);

        EXPECT_TRUE(BE_SQLITE_OK == viewController.Save());
        db->SaveSettings();
        }

    // Read the element back out, modify, and rewrite.
    //.............................................................................................
        {
        DbResult openStatus;
        DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Exclusive));
        ASSERT_TRUE(DGNFILE_STATUS_Success == openStatus);
        ASSERT_TRUE(db.IsValid());
        
        PhysicalTextAnnotationElementCPtr annotationElementC = PhysicalTextAnnotationElement::Get(*db, annotationElement1Id);
        ASSERT_TRUE(annotationElementC.IsValid());

        // Spot check some properties; rely on other TextAnnotation tests to more fully test serialization, which should be relatively pass/fail on the element itself.
        TextAnnotationCP existingAnnotation = annotationElementC->GetAnnotation();
        ASSERT_TRUE(nullptr != existingAnnotation);
        EXPECT_TRUE(nullptr == existingAnnotation->GetFrameCP());
        EXPECT_TRUE(0 == existingAnnotation->GetLeaders().size());

        AnnotationTextBlockCP existingText = existingAnnotation->GetTextCP();
        ASSERT_TRUE(nullptr != existingText);
        EXPECT_TRUE(annotationTextStyle1Id == existingText->GetStyleId());
        
        AnnotationParagraphCollectionCR existingParagraphs = existingText->GetParagraphs();
        ASSERT_TRUE(1 == existingParagraphs.size());
        
        AnnotationRunCollectionCR existingRuns = existingParagraphs[0]->GetRuns();
        ASSERT_TRUE(1 == existingRuns.size());
        
        ASSERT_TRUE(AnnotationRunType::Text == existingRuns[0]->GetType());
        EXPECT_TRUE(0 == strcmp(ANNOTATION_TEXT_1, ((AnnotationTextRunCP)existingRuns[0].get())->GetContent().c_str()));
        
        // Update with different text.
        TextAnnotation annotation(*db);
        annotation.SetText(AnnotationTextBlock::Create(*db, annotationTextStyle1Id, ANNOTATION_TEXT_2).get());

        PhysicalTextAnnotationElementPtr annotationElement = PhysicalTextAnnotationElement::GetForEdit(*db, annotationElement1Id);
        ASSERT_TRUE(annotationElement.IsValid());

        EXPECT_TRUE(SUCCESS == annotationElement->SetAnnotation(annotation));
        
        PhysicalTextAnnotationElementCPtr updatedAnnotationElement = annotationElement->Update();
        ASSERT_TRUE(updatedAnnotationElement.IsValid());
        EXPECT_TRUE(updatedAnnotationElement->GetElementId().IsValid());
        }
    
    // Verify the modified element.
    //.............................................................................................
        {
        DbResult openStatus;
        DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OPEN_ReadWrite, DefaultTxn_Exclusive));
        ASSERT_TRUE(DGNFILE_STATUS_Success == openStatus);
        ASSERT_TRUE(db.IsValid());

        PhysicalTextAnnotationElementCPtr annotationElementC = PhysicalTextAnnotationElement::Get(*db, annotationElement1Id);
        ASSERT_TRUE(annotationElementC.IsValid());

        // Spot check some properties; rely on other TextAnnotation tests to more fully test serialization, which should be relatively pass/fail on the element itself.
        TextAnnotationCP existingAnnotation = annotationElementC->GetAnnotation();
        ASSERT_TRUE(nullptr != existingAnnotation);
        EXPECT_TRUE(nullptr == existingAnnotation->GetFrameCP());
        EXPECT_TRUE(0 == existingAnnotation->GetLeaders().size());

        AnnotationTextBlockCP existingText = existingAnnotation->GetTextCP();
        ASSERT_TRUE(nullptr != existingText);
        EXPECT_TRUE(annotationTextStyle1Id == existingText->GetStyleId());

        AnnotationParagraphCollectionCR existingParagraphs = existingText->GetParagraphs();
        ASSERT_TRUE(1 == existingParagraphs.size());

        AnnotationRunCollectionCR existingRuns = existingParagraphs[0]->GetRuns();
        ASSERT_TRUE(1 == existingRuns.size());

        ASSERT_TRUE(AnnotationRunType::Text == existingRuns[0]->GetType());
        EXPECT_TRUE(0 == strcmp(ANNOTATION_TEXT_2, ((AnnotationTextRunCP)existingRuns[0].get())->GetContent().c_str()));
        }
    }
