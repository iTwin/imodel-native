//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationTextBlock_Test.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextBlockTest : public AnnotationTestFixture
{

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
    OBJ_PTR->GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::ColorValue, 41);\
    OBJ_PTR->GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::IsItalic, 0);\
    OBJ_PTR->GetStyleOverridesR().SetRealProperty(AnnotationTextStyleProperty::WidthFactor, 3.0);\
    EXPECT_TRUE(3 == OBJ_PTR->GetStyleOverrides().ComputePropertyCount());

#define VERIFY_STYLE_OVERRIDES_2(OBJ_PTR)\
    EXPECT_TRUE(3 == OBJ_PTR->GetStyleOverrides().ComputePropertyCount());\
    EXPECT_TRUE(41 == OBJ_PTR->GetStyleOverrides().GetIntegerProperty(AnnotationTextStyleProperty::ColorValue));\
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
    DgnDbR project = *GetDgnDb();

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
    DgnDbR project = *GetDgnDb();

    //.............................................................................................
    DgnFontCR ttFont = DgnFontManager::GetLastResortTrueTypeFont();
    ASSERT_TRUE(ttFont.IsResolved());

    DgnFontId ttFontId = project.Fonts().AcquireId(ttFont);
    ASSERT_TRUE(ttFontId.IsValid());

    AnnotationTextStylePtr docStyle = AnnotationTextStyle::Create(project);
    docStyle->SetName("docStyle");
    docStyle->SetFontId(ttFontId);
    docStyle->SetHeight(11.0);

    docStyle->Insert();
    ASSERT_TRUE(docStyle->GetElementId().IsValid());

    AnnotationTextStylePtr parStyle = docStyle->CreateCopy();
    parStyle->SetName("parStyle");
    parStyle->Insert();
    ASSERT_TRUE(parStyle->GetElementId().IsValid());

    AnnotationTextStylePtr runStyle = docStyle->CreateCopy();
    runStyle->SetName("runStyle");
    runStyle->Insert();
    ASSERT_TRUE(runStyle->GetElementId().IsValid());

    //.............................................................................................
    AnnotationTextRunPtr textRun = AnnotationTextRun::Create(project);
    textRun->SetStyleId(runStyle->GetElementId(), SetAnnotationTextStyleOptions::Default);
    SET_STYLE_OVERRIDES_1(textRun);
    DECLARE_AND_SET_TEXTRUN_DATA_1(textRun,textRun);

    AnnotationLineBreakRunPtr brkRun = AnnotationLineBreakRun::Create(project);
    brkRun->SetStyleId(runStyle->GetElementId(), SetAnnotationTextStyleOptions::Default);

    AnnotationFractionRunPtr fracRun = AnnotationFractionRun::Create(project);
    fracRun->SetStyleId(runStyle->GetElementId(), SetAnnotationTextStyleOptions::Default);
    SET_STYLE_OVERRIDES_2(fracRun);
    DECLARE_AND_SET_FRACRUN_DATA_1(fracRun,fracRun);

    AnnotationParagraphPtr par1 = AnnotationParagraph::Create(project);
    par1->SetStyleId(parStyle->GetElementId(), SetAnnotationTextStyleOptions::Default);
    par1->GetRunsR().push_back(textRun);
    par1->GetRunsR().push_back(brkRun);

    AnnotationParagraphPtr par2 = AnnotationParagraph::Create(project);
    par2->SetStyleId(parStyle->GetElementId(), SetAnnotationTextStyleOptions::Default);
    par2->GetRunsR().push_back(fracRun);

    AnnotationTextBlockPtr doc = AnnotationTextBlock::Create(project);
    doc->SetStyleId(docStyle->GetElementId(), SetAnnotationTextStyleOptions::Default);
    SET_STYLE_OVERRIDES_3(doc);
    DECLARE_AND_SET_DOC_DATA_1(doc,doc);

    doc->GetParagraphsR().push_back(par1);
    doc->GetParagraphsR().push_back(par2);

    //.............................................................................................
    AnnotationTextBlockPtr clonedDoc = doc->Clone();
    ASSERT_TRUE(clonedDoc.IsValid());
    EXPECT_TRUE(docStyle->GetElementId() == clonedDoc->GetStyleId());
    VERIFY_STYLE_OVERRIDES_3(clonedDoc);
    VERIFY_DOC_DATA_1(clonedDoc,doc);
    ASSERT_TRUE(2 == clonedDoc->GetParagraphs().size());

    AnnotationParagraphPtr clonedPar1 = clonedDoc->GetParagraphs()[0];
    EXPECT_TRUE(parStyle->GetElementId() == clonedPar1->GetStyleId());
    ASSERT_TRUE(2 == clonedPar1->GetRuns().size());

    AnnotationRunBasePtr clonedRun1 = clonedPar1->GetRuns()[0];
    EXPECT_TRUE(runStyle->GetElementId() == clonedRun1->GetStyleId());
    VERIFY_STYLE_OVERRIDES_1(clonedRun1);
    AnnotationTextRunCP clonedTextRun = (AnnotationTextRunCP)clonedRun1.get();
    VERIFY_TEXTRUN_DATA_1(clonedTextRun,textRun);

    AnnotationRunBasePtr clonedRun2 = clonedPar1->GetRuns()[1];
    EXPECT_TRUE(runStyle->GetElementId() == clonedRun2->GetStyleId());

    AnnotationParagraphPtr clonedPar2 = clonedDoc->GetParagraphs()[1];
    EXPECT_TRUE(parStyle->GetElementId() == clonedPar2->GetStyleId());
    ASSERT_TRUE(1 == clonedPar2->GetRuns().size());

    AnnotationRunBasePtr clonedRun3 = clonedPar2->GetRuns()[0];
    EXPECT_TRUE(runStyle->GetElementId() == clonedRun3->GetStyleId());
    VERIFY_STYLE_OVERRIDES_2(clonedRun3);
    AnnotationFractionRunCP clonedFracRun = (AnnotationFractionRunCP)clonedRun3.get();
    VERIFY_FRACRUN_DATA_1(clonedFracRun,fracRun);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat     07/2015
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextBlockTest, CreateAnnotationTextBlock)
{
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

    //.............................................................................................
    AnnotationTextBlockPtr doc = AnnotationTextBlock::Create(project);
    ASSERT_TRUE(doc.IsValid());

    // Basics
    EXPECT_TRUE(&project == &doc->GetDbR());
    EXPECT_TRUE(0 == doc->GetParagraphs().size());

    //.............................................................................................
    // Text Style
    AnnotationTextStylePtr testStyle = AnnotationTextStyle::Create(project);
    ASSERT_TRUE(testStyle.IsValid());
    
    /*
    //.............................................................................................
    // Create with invalid style id
    // Why does it create AnnotationTextBlock with invalid style id ?
    AnnotationTextBlockPtr doc2 = AnnotationTextBlock::Create(project, testStyle->GetStyleId());
    ASSERT_FALSE(doc2.IsValid());*/

    testStyle->SetName("my style");
    ASSERT_TRUE(testStyle->Insert().IsValid());
    ASSERT_TRUE(testStyle->GetElementId().IsValid());

    //.............................................................................................
    // Create with Valid style id
    AnnotationTextBlockPtr doc3 = AnnotationTextBlock::Create(project,testStyle->GetElementId());
    ASSERT_TRUE(doc3.IsValid());

    // Basic Check
    EXPECT_TRUE(&project == &doc3->GetDbR());
    EXPECT_TRUE(0 == doc3->GetParagraphs().size());
    EXPECT_TRUE(testStyle->GetElementId() == doc3->GetStyleId());

    //.............................................................................................
    // Create Annotation Paragraph
    AnnotationTextRunPtr run = createAnnotationTextRun(project, testStyle);
    AnnotationParagraphPtr para = createAnnotationParagraph(project, testStyle, run);
    para->SetStyleId(testStyle->GetElementId(), SetAnnotationTextStyleOptions::Direct);

    //.............................................................................................
    // Create with annotation paragraph
    AnnotationTextBlockPtr doc5 = AnnotationTextBlock::Create(project,testStyle->GetElementId(),*para);
    ASSERT_TRUE(doc5.IsValid());

    // Basics Checks
    EXPECT_TRUE(&project == &doc5->GetDbR());
    EXPECT_TRUE(1 == doc5->GetParagraphs().size());
    EXPECT_TRUE(testStyle->GetElementId() == doc5->GetStyleId());

    //.............................................................................................
    // Create with contents
    const char * contents = "Contents";
    AnnotationTextBlockPtr doc6 = AnnotationTextBlock::Create(project, testStyle->GetElementId(), contents);
    ASSERT_TRUE(doc6.IsValid());

    // Basics Checks
    EXPECT_TRUE(&project == &doc6->GetDbR());
    EXPECT_TRUE(1 == doc6->GetParagraphs().size());
    EXPECT_TRUE(testStyle->GetElementId() == doc6->GetStyleId());
    ASSERT_TRUE(1 == doc6->GetParagraphsR().at(0)->GetRunsR().size());
    AnnotationTextRunP textRun = dynamic_cast<AnnotationTextRunP>(doc6->GetParagraphsR().at(0)->GetRunsR().at(0).get());
    ASSERT_STREQ(contents, textRun->GetContent().c_str());

    //.............................................................................................
    // Create with Constructor
    AnnotationTextBlock doc4(project);

    // Basics Check
    EXPECT_TRUE(&project == &doc4.GetDbR());
    EXPECT_TRUE(0 == doc4.GetParagraphs().size());

    // AppendParagraph
    AnnotationTextBlockPtr doc7 = AnnotationTextBlock::Create(project);
    ASSERT_TRUE(doc7.IsValid());
    EXPECT_TRUE(&project == &doc7->GetDbR());
    EXPECT_TRUE(0 == doc7->GetParagraphs().size());

    doc7->AppendParagraph();
    EXPECT_TRUE(1 == doc7->GetParagraphs().size());

}
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat     07/2015
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextBlockTest, Unicode)
{
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

    //.............................................................................................
    // Text Style
    AnnotationTextStylePtr testStyle = AnnotationTextStyle::Create(project);
    ASSERT_TRUE(testStyle.IsValid());

    AnnotationTextRunPtr run = createAnnotationTextRun(project, testStyle);
    AnnotationLineBreakRunPtr brkRun = AnnotationLineBreakRun::Create(project, testStyle->GetElementId());
    ASSERT_TRUE(brkRun.IsValid());
    AnnotationParagraphPtr para = createAnnotationParagraph(project, testStyle, run);
    para->GetRunsR().push_back(brkRun);

    //.............................................................................................
    // Create with contents

    WString contentStr = L"عمر حیات";
    char * contents = new char[contentStr.length() + 1];
    contentStr.ConvertToLocaleChars(contents);
    AnnotationTextBlockPtr doc6 = AnnotationTextBlock::Create(project, testStyle->GetElementId(), contents);
    ASSERT_TRUE(doc6.IsValid());
    
    EXPECT_TRUE(testStyle->GetElementId() == doc6->GetStyleId());
    ASSERT_EQ(1 , doc6->GetParagraphsR().at(0)->GetRunsR().size());
    AnnotationTextRunP textRun = dynamic_cast<AnnotationTextRunP>(doc6->GetParagraphsR().at(0)->GetRunsR().at(0).get());
    ASSERT_STREQ(contents, textRun->GetContent().c_str());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2015
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextBlockTest, ToString)
    {
    DgnDbR db = *GetDgnDb();

    AnnotationTextStylePtr style = AnnotationTextStyle::Create(db);
    style->SetName("style");
    AnnotationTextStyleCPtr fileStyle = style->Insert();
    DgnElementId fileStyleId = fileStyle->GetElementId();
    
    static const Utf8String STR_1("Lorem ipsum dolar sit amet.");
    AnnotationTextBlockPtr text = AnnotationTextBlock::Create(db, fileStyleId, STR_1.c_str());
    Utf8String textStr = text->ToString();
    
    EXPECT_TRUE(0 == strcmp(STR_1.c_str(), textStr.c_str()));

    static const Utf8String STR_2("Consectetur adipiscing elit. ");
    static const Utf8String STR_3("1");
    static const Utf8String STR_4("2");
    static const Utf8String STR_5(" Vivamus dictum interdum tellus.");

    AnnotationParagraphPtr para2 = AnnotationParagraph::Create(db, fileStyleId);
    para2->GetRunsR().push_back(AnnotationTextRun::Create(db, fileStyleId, STR_2.c_str()));
    para2->GetRunsR().push_back(AnnotationFractionRun::Create(db, fileStyleId, STR_3.c_str(), STR_4.c_str()));
    para2->GetRunsR().push_back(AnnotationTextRun::Create(db, fileStyleId, STR_5.c_str()));

    text->GetParagraphsR().push_back(para2);

    Utf8PrintfString expectedString("%s %s%s/%s%s", STR_1.c_str(), STR_2.c_str(), STR_3.c_str(), STR_4.c_str(), STR_5.c_str());
    textStr = text->ToString();
    
    EXPECT_TRUE(0 == strcmp(expectedString.c_str(), textStr.c_str()));
    }
