//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationParagraph_Test.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationParagraphTest : public AnnotationTestFixture
{

}; // AnnotationParagraphTest

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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TEST_F(AnnotationParagraphTest, TextRun)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

    AnnotationTextStylePtr style = AnnotationTestFixture::createAnnotationTextStyle(project, "RunStyle");
    ASSERT_TRUE(style.IsValid());

    //.............................................................................................
    AnnotationTextRunPtr textRun1 = AnnotationTextRun::Create(project);
    ASSERT_TRUE(textRun1.IsValid());
    AnnotationTextRunPtr textRun2 = AnnotationTextRun::Create(project, style->GetElementId());
    ASSERT_TRUE(textRun2.IsValid());
    AnnotationTextRunPtr textRun3 = AnnotationTextRun::Create(project, style->GetElementId(),"Run Contetents");
    ASSERT_TRUE(textRun3.IsValid());

    // Basics
    EXPECT_TRUE(&project == &textRun1->GetDbR());
    EXPECT_TRUE(&project == &textRun2->GetDbR());
    EXPECT_TRUE(&project == &textRun3->GetDbR());

    // Defaults
    EXPECT_TRUE(!textRun1->GetStyleId().IsValid());
    EXPECT_TRUE(0 == textRun1->GetStyleOverrides().ComputePropertyCount());
    EXPECT_TRUE(textRun1->GetContent().empty());
    EXPECT_TRUE(!textRun1->IsSubScript());
    EXPECT_TRUE(!textRun1->IsSuperScript());

    // Set/Get round-trip
    DECLARE_AND_SET_TEXTRUN_DATA_1(textRun1,textRun1);
    VERIFY_TEXTRUN_DATA_1(textRun1,textRun1);

    textRun2->SetSubSuperScript(AnnotationTextRunSubSuperScript::SubScript);
    EXPECT_TRUE(textRun2->IsSubScript());
    EXPECT_TRUE(textRun2->GetStyleId().IsValid());

    textRun3->SetSubSuperScript(AnnotationTextRunSubSuperScript::SuperScript);
    EXPECT_TRUE(textRun3->IsSuperScript());
    EXPECT_TRUE(textRun3->GetStyleId().IsValid());
    EXPECT_STREQ("Run Contetents",textRun3->GetContent().c_str());

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TEST_F(AnnotationParagraphTest, FractionRun)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();
    AnnotationTextStylePtr style = AnnotationTestFixture::createAnnotationTextStyle(project, "RunStyle");
    ASSERT_TRUE(style.IsValid());

    //.............................................................................................
    AnnotationFractionRunPtr fracRun = AnnotationFractionRun::Create(project);
    ASSERT_TRUE(fracRun.IsValid());
    AnnotationFractionRunPtr fracRun2 = AnnotationFractionRun::Create(project, style->GetElementId());
    ASSERT_TRUE(fracRun2.IsValid());
    AnnotationFractionRunPtr fracRun3 = AnnotationFractionRun::Create(project, style->GetElementId(), "MyNumerContent", "MyDenomContent");
    ASSERT_TRUE(fracRun3.IsValid());

    // Basics
    EXPECT_TRUE(&project == &fracRun->GetDbR());
    EXPECT_TRUE(&project == &fracRun2->GetDbR());
    EXPECT_TRUE(&project == &fracRun3->GetDbR());

    // Defaults
    EXPECT_TRUE(!fracRun->GetStyleId().IsValid());
    EXPECT_TRUE(0 == fracRun->GetStyleOverrides().ComputePropertyCount());
    EXPECT_TRUE(fracRun->GetDenominatorContent().empty());
    EXPECT_TRUE(fracRun->GetNumeratorContent().empty());

    // Set/Get round-trip
    DECLARE_AND_SET_FRACRUN_DATA_1(fracRun,fracRun);
    VERIFY_FRACRUN_DATA_1(fracRun,fracRun);

    EXPECT_TRUE(fracRun3->GetStyleId().IsValid());
    EXPECT_TRUE(0 == fracRun3->GetStyleOverrides().ComputePropertyCount());
    EXPECT_STREQ("MyNumerContent", fracRun3->GetNumeratorContent().c_str());
    EXPECT_STREQ("MyDenomContent", fracRun3->GetDenominatorContent().c_str());

    AnnotationFractionRunPtr fracRun4 = fracRun3->CloneAsFractionRun();
    ASSERT_TRUE(fracRun4.IsValid());
    EXPECT_TRUE(&project == &fracRun4->GetDbR());
    EXPECT_TRUE(fracRun4->GetStyleId().IsValid());
    EXPECT_TRUE(0 == fracRun4->GetStyleOverrides().ComputePropertyCount());
    EXPECT_STREQ("MyNumerContent", fracRun4->GetNumeratorContent().c_str());
    EXPECT_STREQ("MyDenomContent", fracRun4->GetDenominatorContent().c_str());
    
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TEST_F(AnnotationParagraphTest, LineBreakRun)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();
    AnnotationTextStylePtr style = AnnotationTestFixture::createAnnotationTextStyle(project, "RunStyle");
    ASSERT_TRUE(style.IsValid());

    //.............................................................................................
    AnnotationLineBreakRunPtr brkRun1 = AnnotationLineBreakRun::Create(project);
    ASSERT_TRUE(brkRun1.IsValid());
    AnnotationLineBreakRunPtr brkRun2 = AnnotationLineBreakRun::Create(project,style->GetElementId());
    ASSERT_TRUE(brkRun2.IsValid());

    // brkRun 1
    EXPECT_TRUE(&project == &brkRun1->GetDbR());
    EXPECT_TRUE(!brkRun1->GetStyleId().IsValid());
    EXPECT_TRUE(0 == brkRun1->GetStyleOverrides().ComputePropertyCount());

    // brkRun 2
    EXPECT_TRUE(&project == &brkRun2->GetDbR());
    EXPECT_TRUE(brkRun2->GetStyleId().IsValid());
    EXPECT_TRUE(0 == brkRun2->GetStyleOverrides().ComputePropertyCount());

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TEST_F(AnnotationParagraphTest, AnnotationParagraph)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();
    AnnotationTextStylePtr style = AnnotationTestFixture::createAnnotationTextStyle(project, "ParaStyle");
    ASSERT_TRUE(style.IsValid());
    AnnotationTextRunPtr textRun = AnnotationTextRun::Create(project, style->GetElementId(), "PI is ");
    ASSERT_TRUE(textRun.IsValid());
    AnnotationFractionRunPtr fracRun = AnnotationFractionRun::Create(project, style->GetElementId(), "22", "7");
    ASSERT_TRUE(fracRun.IsValid());
    AnnotationLineBreakRunPtr brkRun = AnnotationLineBreakRun::Create(project, style->GetElementId());
    ASSERT_TRUE(brkRun.IsValid());

    //.............................................................................................
    AnnotationParagraphPtr para1 = AnnotationParagraph::Create(project);
    ASSERT_TRUE(para1.IsValid());
    AnnotationParagraphPtr para2 = AnnotationParagraph::Create(project, style->GetElementId());
    ASSERT_TRUE(para2.IsValid());
    AnnotationParagraphPtr para3 = AnnotationParagraph::Create(project, style->GetElementId(), *textRun);
    ASSERT_TRUE(para3.IsValid());

    // para1
    EXPECT_TRUE(&project == &para1->GetDbR());
    EXPECT_TRUE(0 == para1->GetRuns().size());
    EXPECT_TRUE(!para1->GetStyleId().IsValid());
    EXPECT_TRUE(0 == para1->GetStyleOverrides().ComputePropertyCount());

    // para 2
    EXPECT_TRUE(&project == &para2->GetDbR());
    EXPECT_TRUE(0 == para2->GetRuns().size());
    EXPECT_TRUE(para2->GetStyleId().IsValid());
    EXPECT_TRUE(0 == para2->GetStyleOverrides().ComputePropertyCount());

    // para 3
    EXPECT_TRUE(&project == &para3->GetDbR());
    EXPECT_TRUE(1 == para3->GetRuns().size());
    EXPECT_TRUE(para3->GetStyleId().IsValid());
    EXPECT_TRUE(0 == para3->GetStyleOverrides().ComputePropertyCount());

    // Append Run
    para1->SetStyleId(style->GetElementId(), SetAnnotationTextStyleOptions::Default);
    para1->AppendRun(*textRun);
    para1->AppendRun(*fracRun);
    para1->AppendRun(*brkRun);
    EXPECT_TRUE(3 == para1->GetRuns().size());
    
    }
