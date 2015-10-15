//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationTestFixture.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr AnnotationTestFixture::createAnnotationTextStyle(DgnDbR project , const char* styleName)
    {
    //.............................................................................................
    DgnFontCR ttFont = DgnFontManager::GetLastResortTrueTypeFont();
    EXPECT_TRUE(ttFont.IsResolved());

    DgnFontId ttFontId = project.Fonts().AcquireId(ttFont);
    EXPECT_TRUE(ttFontId.IsValid());

    AnnotationTextStylePtr docStyle = AnnotationTextStyle::Create(project);
    docStyle->SetName(styleName);
    docStyle->SetFontId(ttFontId);
    docStyle->SetHeight(11.0);
    
    EXPECT_TRUE(docStyle->Insert().IsValid());
    EXPECT_TRUE(docStyle->GetStyleId().IsValid());
    
    return docStyle;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
AnnotationTextRunPtr AnnotationTestFixture::createAnnotationTextRun(DgnDbR project , AnnotationTextStylePtr runStyle)
    {
        AnnotationTextRunPtr textRun = AnnotationTextRun::Create(project);
    textRun->SetStyleId(runStyle->GetStyleId(), SetAnnotationTextStyleOptions::Default);
    return textRun;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
AnnotationParagraphPtr AnnotationTestFixture::createAnnotationParagraph(DgnDbR project , AnnotationTextStylePtr parStyle, AnnotationTextRunPtr textRun)
    {
    AnnotationParagraphPtr par1 = AnnotationParagraph::Create(project);
    par1->SetStyleId(parStyle->GetStyleId(), SetAnnotationTextStyleOptions::Default);
    par1->GetRunsR().push_back(textRun);
    return par1;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
AnnotationTextBlockPtr AnnotationTestFixture::createAnnotationTextBlock(DgnDbR project , AnnotationTextStylePtr docStyle, AnnotationParagraphPtr par1)
    {
    AnnotationTextBlockPtr doc = AnnotationTextBlock::Create(project);
    doc->SetStyleId(docStyle->GetStyleId(), SetAnnotationTextStyleOptions::Default);    
    doc->GetParagraphsR().push_back(par1);
    return doc;
}
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePtr AnnotationTestFixture::createAnnotationLeaderStyle(DgnDbR project, const char* styleName)
    {
        //.............................................................................................
        AnnotationLeaderStylePtr style = AnnotationLeaderStyle::Create(project);
        EXPECT_TRUE(style.IsValid());
        style->SetName(styleName);

        EXPECT_TRUE(style->Insert().IsValid());
        EXPECT_TRUE(style->GetStyleId().IsValid());
        return style;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
AnnotationFrameStylePtr AnnotationTestFixture::createAnnotationFrameStyle(DgnDbR project, const char* styleName)
    {
        //.............................................................................................
        AnnotationFrameStylePtr style = AnnotationFrameStyle::Create(project);
        EXPECT_TRUE(style.IsValid());

        style->SetName(styleName);
        EXPECT_TRUE(style->Insert().IsValid());
        EXPECT_TRUE(style->GetStyleId().IsValid());
        return style;
    }
