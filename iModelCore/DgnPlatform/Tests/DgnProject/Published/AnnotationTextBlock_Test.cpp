//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationTextBlock_Test.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
class AnnotationTextBlockTest : public GenericDgnModelTestFixture
{
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     05/2014
    //---------------------------------------------------------------------------------------
    public: AnnotationTextBlockTest () :
        GenericDgnModelTestFixture (__FILE__, false /*2D*/)
        {
        }

}; // AnnotationTextBlockTest

#define DECLARE_AND_SET_DOC_DATA_1(DOC_PTR,VAR_PFX)\
    double VAR_PFX##DocumentWidth = 11.0;                                               DOC_PTR->SetDocumentWidth(VAR_PFX##DocumentWidth);\
    auto VAR_PFX##Justification = AnnotationTextBlock::HorizontalJustification::Center; DOC_PTR->SetJustification(VAR_PFX##Justification);

#define VERIFY_DOC_DATA_1(DOC_PTR,VAR_PFX)\
    EXPECT_TRUE(VAR_PFX##DocumentWidth == DOC_PTR->GetDocumentWidth());\
    EXPECT_TRUE(VAR_PFX##Justification == DOC_PTR->GetJustification());

#define DECLARE_AND_SET_TEXTRUN_DATA_1(RUN_PTR,VAR_PFX)\
    Utf8String VAR_PFX##Content = "MyContent";  RUN_PTR->SetContent(VAR_PFX##Content.c_str());

#define VERIFY_TEXTRUN_DATA_1(RUN_PTR,VAR_PFX)\
    EXPECT_TRUE(VAR_PFX##Content.Equals(RUN_PTR->GetContent()));

#define DECLARE_AND_SET_FRACRUN_DATA_1(RUN_PTR,VAR_PFX)\
    Utf8String VAR_PFX##DenominatorContent = "MyDenomContent";  RUN_PTR->SetDenominatorContent(VAR_PFX##DenominatorContent.c_str());\
    Utf8String VAR_PFX##NumeratorContent = "MyNumerContent";    RUN_PTR->SetNumeratorContent(VAR_PFX##NumeratorContent.c_str());

#define VERIFY_FRACRUN_DATA_1(RUN_PTR,VAR_PFX)\
    EXPECT_TRUE(VAR_PFX##DenominatorContent.Equals(RUN_PTR->GetDenominatorContent()));\
    EXPECT_TRUE(VAR_PFX##NumeratorContent.Equals(RUN_PTR->GetNumeratorContent()));

#define SET_STYLE_OVERRIDES_1(OBJ_PTR)\
    OBJ_PTR->GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::IsBold, 1);\
    OBJ_PTR->GetStyleOverridesR().SetRealProperty(AnnotationTextStyleProperty::WidthFactor, 2.5);\
    EXPECT_TRUE(2 == OBJ_PTR->GetStyleOverrides().ComputePropertyCount());

#define VERIFY_STYLE_OVERRIDES_1(OBJ_PTR)\
    EXPECT_TRUE(2 == OBJ_PTR->GetStyleOverrides().ComputePropertyCount());\
    EXPECT_TRUE(1 == OBJ_PTR->GetStyleOverrides().GetIntegerProperty(AnnotationTextStyleProperty::IsBold));\
    EXPECT_TRUE(2.5 == OBJ_PTR->GetStyleOverrides().GetRealProperty(AnnotationTextStyleProperty::WidthFactor));

#define SET_STYLE_OVERRIDES_2(OBJ_PTR)\
    OBJ_PTR->GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::ColorId, 41);\
    OBJ_PTR->GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::IsItalic, 0);\
    OBJ_PTR->GetStyleOverridesR().SetRealProperty(AnnotationTextStyleProperty::WidthFactor, 3.0);\
    EXPECT_TRUE(3 == OBJ_PTR->GetStyleOverrides().ComputePropertyCount());

#define VERIFY_STYLE_OVERRIDES_2(OBJ_PTR)\
    EXPECT_TRUE(3 == OBJ_PTR->GetStyleOverrides().ComputePropertyCount());\
    EXPECT_TRUE(41 == OBJ_PTR->GetStyleOverrides().GetIntegerProperty(AnnotationTextStyleProperty::ColorId));\
    EXPECT_TRUE(0 == OBJ_PTR->GetStyleOverrides().GetIntegerProperty(AnnotationTextStyleProperty::IsItalic));\
    EXPECT_TRUE(3.0 == OBJ_PTR->GetStyleOverrides().GetRealProperty(AnnotationTextStyleProperty::WidthFactor));

#define SET_STYLE_OVERRIDES_3(OBJ_PTR)\
    OBJ_PTR->GetStyleOverridesR().SetRealProperty(AnnotationTextStyleProperty::Height, 21.0);\
    EXPECT_TRUE(1 == OBJ_PTR->GetStyleOverrides().ComputePropertyCount());

#define VERIFY_STYLE_OVERRIDES_3(OBJ_PTR)\
    EXPECT_TRUE(1 == OBJ_PTR->GetStyleOverrides().ComputePropertyCount());\
    EXPECT_TRUE(21.0 == OBJ_PTR->GetStyleOverrides().GetRealProperty(AnnotationTextStyleProperty::Height));

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextBlockTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    AnnotationTextBlockPtr doc = AnnotationTextBlock::Create(project);
    ASSERT_TRUE(doc.IsValid());

    // Basics
    EXPECT_TRUE(&project == &doc->GetDbR());
    EXPECT_TRUE(0 == doc->GetParagraphs().size());
    
    // Defaults
    EXPECT_TRUE(0.0 == doc->GetDocumentWidth());
    EXPECT_TRUE(AnnotationTextBlock::HorizontalJustification::Left == doc->GetJustification());
    EXPECT_TRUE(!doc->GetStyleId().IsValid());
    EXPECT_TRUE(0 == doc->GetStyleOverrides().ComputePropertyCount());

    // Set/Get round-trip
    DECLARE_AND_SET_DOC_DATA_1(doc,doc);
    VERIFY_DOC_DATA_1(doc,doc);

    //.............................................................................................
    AnnotationParagraphPtr par = AnnotationParagraph::Create(project);
    ASSERT_TRUE(par.IsValid());

    // Basics
    EXPECT_TRUE(&project == &par->GetDbR());
    EXPECT_TRUE(0 == par->GetRuns().size());

    // Defaults
    EXPECT_TRUE(!par->GetStyleId().IsValid());
    EXPECT_TRUE(0 == par->GetStyleOverrides().ComputePropertyCount());

    // Nothing to get/set round-trip.

    //.............................................................................................
    AnnotationTextRunPtr textRun = AnnotationTextRun::Create(project);
    ASSERT_TRUE(textRun.IsValid());

    // Basics
    EXPECT_TRUE(&project == &textRun->GetDbR());

    // Defaults
    EXPECT_TRUE(!textRun->GetStyleId().IsValid());
    EXPECT_TRUE(0 == textRun->GetStyleOverrides().ComputePropertyCount());
    EXPECT_TRUE(textRun->GetContent().empty());

    // Set/Get round-trip
    DECLARE_AND_SET_TEXTRUN_DATA_1(textRun,textRun);
    VERIFY_TEXTRUN_DATA_1(textRun,textRun);
    
    //.............................................................................................
    AnnotationFractionRunPtr fracRun = AnnotationFractionRun::Create(project);
    ASSERT_TRUE(fracRun.IsValid());

    // Basics
    EXPECT_TRUE(&project == &fracRun->GetDbR());

    // Defaults
    EXPECT_TRUE(!fracRun->GetStyleId().IsValid());
    EXPECT_TRUE(0 == fracRun->GetStyleOverrides().ComputePropertyCount());
    EXPECT_TRUE(fracRun->GetDenominatorContent().empty());
    EXPECT_TRUE(fracRun->GetNumeratorContent().empty());

    // Set/Get round-trip
    DECLARE_AND_SET_FRACRUN_DATA_1(fracRun,fracRun);
    VERIFY_FRACRUN_DATA_1(fracRun,fracRun);
    
    //.............................................................................................
    AnnotationLineBreakRunPtr brkRun = AnnotationLineBreakRun::Create(project);
    ASSERT_TRUE(brkRun.IsValid());

    // Basics
    EXPECT_TRUE(&project == &brkRun->GetDbR());

    // Defaults
    EXPECT_TRUE(!brkRun->GetStyleId().IsValid());
    EXPECT_TRUE(0 == brkRun->GetStyleOverrides().ComputePropertyCount());

    // Nothing to get/set round-trip.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextBlockTest, DeepCopy)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    DgnFontCR ttFont = DgnFontManager::GetLastResortTrueTypeFont();
    ASSERT_TRUE(ttFont.IsResolved());

    DgnFontId ttFontId = project.Fonts().AcquireId(ttFont);
    ASSERT_TRUE(ttFontId.IsValid());

    AnnotationTextStylePtr docStyle = AnnotationTextStyle::Create(project);
    docStyle->SetName("docStyle");
    docStyle->SetFontId(ttFontId);
    docStyle->SetHeight(11.0);

    project.Styles().AnnotationTextStyles().Insert(*docStyle);
    ASSERT_TRUE(docStyle.IsValid());
    ASSERT_TRUE(docStyle->GetId().IsValid());

    AnnotationTextStylePtr parStyle = docStyle->Clone();
    parStyle->SetName("parStyle");
    project.Styles().AnnotationTextStyles().Insert(*parStyle);
    ASSERT_TRUE(parStyle.IsValid());
    ASSERT_TRUE(parStyle->GetId().IsValid());

    AnnotationTextStylePtr runStyle = docStyle->Clone();
    runStyle->SetName("runStyle");
    project.Styles().AnnotationTextStyles().Insert(*runStyle);
    ASSERT_TRUE(runStyle.IsValid());
    ASSERT_TRUE(runStyle->GetId().IsValid());

    //.............................................................................................
    AnnotationTextRunPtr textRun = AnnotationTextRun::Create(project);
    textRun->SetStyleId(runStyle->GetId(), SetAnnotationTextStyleOptions::Default);
    SET_STYLE_OVERRIDES_1(textRun);
    DECLARE_AND_SET_TEXTRUN_DATA_1(textRun,textRun);

    AnnotationLineBreakRunPtr brkRun = AnnotationLineBreakRun::Create(project);
    brkRun->SetStyleId(runStyle->GetId(), SetAnnotationTextStyleOptions::Default);

    AnnotationFractionRunPtr fracRun = AnnotationFractionRun::Create(project);
    fracRun->SetStyleId(runStyle->GetId(), SetAnnotationTextStyleOptions::Default);
    SET_STYLE_OVERRIDES_2(fracRun);
    DECLARE_AND_SET_FRACRUN_DATA_1(fracRun,fracRun);

    AnnotationParagraphPtr par1 = AnnotationParagraph::Create(project);
    par1->SetStyleId(parStyle->GetId(), SetAnnotationTextStyleOptions::Default);
    par1->GetRunsR().push_back(textRun);
    par1->GetRunsR().push_back(brkRun);

    AnnotationParagraphPtr par2 = AnnotationParagraph::Create(project);
    par2->SetStyleId(parStyle->GetId(), SetAnnotationTextStyleOptions::Default);
    par2->GetRunsR().push_back(fracRun);

    AnnotationTextBlockPtr doc = AnnotationTextBlock::Create(project);
    doc->SetStyleId(docStyle->GetId(), SetAnnotationTextStyleOptions::Default);
    SET_STYLE_OVERRIDES_3(doc);
    DECLARE_AND_SET_DOC_DATA_1(doc,doc);

    doc->GetParagraphsR().push_back(par1);
    doc->GetParagraphsR().push_back(par2);

    //.............................................................................................
    AnnotationTextBlockPtr clonedDoc = doc->Clone();
    ASSERT_TRUE(clonedDoc.IsValid());
    EXPECT_TRUE(docStyle->GetId() == clonedDoc->GetStyleId());
    VERIFY_STYLE_OVERRIDES_3(clonedDoc);
    VERIFY_DOC_DATA_1(clonedDoc,doc);
    ASSERT_TRUE(2 == clonedDoc->GetParagraphs().size());

    AnnotationParagraphPtr clonedPar1 = clonedDoc->GetParagraphs()[0];
    EXPECT_TRUE(parStyle->GetId() == clonedPar1->GetStyleId());
    ASSERT_TRUE(2 == clonedPar1->GetRuns().size());

    AnnotationRunBasePtr clonedRun1 = clonedPar1->GetRuns()[0];
    EXPECT_TRUE(runStyle->GetId() == clonedRun1->GetStyleId());
    VERIFY_STYLE_OVERRIDES_1(clonedRun1);
    AnnotationTextRunCP clonedTextRun = (AnnotationTextRunCP)clonedRun1.get();
    VERIFY_TEXTRUN_DATA_1(clonedTextRun,textRun);

    AnnotationRunBasePtr clonedRun2 = clonedPar1->GetRuns()[1];
    EXPECT_TRUE(runStyle->GetId() == clonedRun2->GetStyleId());

    AnnotationParagraphPtr clonedPar2 = clonedDoc->GetParagraphs()[1];
    EXPECT_TRUE(parStyle->GetId() == clonedPar2->GetStyleId());
    ASSERT_TRUE(1 == clonedPar2->GetRuns().size());

    AnnotationRunBasePtr clonedRun3 = clonedPar2->GetRuns()[0];
    EXPECT_TRUE(runStyle->GetId() == clonedRun3->GetStyleId());
    VERIFY_STYLE_OVERRIDES_2(clonedRun3);
    AnnotationFractionRunCP clonedFracRun = (AnnotationFractionRunCP)clonedRun3.get();
    VERIFY_FRACRUN_DATA_1(clonedFracRun,fracRun);
    }
